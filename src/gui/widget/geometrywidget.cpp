/** @file geometrywidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include "geometrywidget.h"
#include "gl/vertexarrayobject.h"
#include "gl/drawable.h"
#include "gl/shadersource.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/lightsettings.h"
#include "gl/compatibility.h"
#include "geom/geometry.h"
#include "io/settings.h"

#include "gl/opengl_undef.h"

namespace MO {
namespace GUI {


GeometryWidget::GeometryWidget(RenderMode mode, QWidget *parent) :
    Basic3DWidget   (mode, parent),
    glprops_        (0),
    drawable_       (new GL::Drawable("geomwidget")),
    tex_            (0),
    texNorm_        (0),
    lights_         (new GL::LightSettings()),
    showGrid_       (settings()->value("GeometryWidget/showGrid", false).toBool()),
    showTexture_    (settings()->value("GeometryWidget/showTexture", false).toBool()),
    showNormalMap_  (settings()->value("GeometryWidget/showNormalMap", false).toBool()),
    showLights_     (settings()->value("GeometryWidget/showLights", false).toBool()),
    pointsize_      (settings()->value("GeometryWidget/pointsize", 4).toInt())
{
    setMinimumSize(128, 128);

    setLights_(showLights_ ? 0.8f : 1.f, Mat4(1));
}

void GeometryWidget::setLights_(Float amp, const Mat4& trans)
{
    lights_->resize(3);
    Vec4 pos = trans * Vec4(1000.f, 2000.f, 800.f, 1.f);
    lights_->setPosition(0, pos.x, pos.y, pos.z);
    pos = trans * Vec4(-2000.f, 1000.f, 1200.f, 1.f);
    lights_->setPosition(1, pos.x, pos.y, pos.z);
    pos = trans * Vec4(2000.f, -500.f, 1500.f, 1.f);
    lights_->setPosition(2, pos.x, pos.y, pos.z);
    lights_->setColor(0, 0.7f*amp, 0.7f*amp, 0.7f*amp);
    lights_->setColor(1, 0.2f*amp, 0.5f*amp, 0.8f*amp);
    lights_->setColor(2, 0.5f*amp, 0.25f*amp, 0.1f*amp);
    lights_->setDiffuseExponent(0, 4);
    lights_->setDiffuseExponent(1, 3);
    lights_->setDiffuseExponent(2, 2);
}

GeometryWidget::~GeometryWidget()
{
    settings()->setValue("GeometryWidget/showGrid", showGrid_);
    settings()->setValue("GeometryWidget/showTexture", showTexture_);
    settings()->setValue("GeometryWidget/showNormalMap", showNormalMap_);
    settings()->setValue("GeometryWidget/showLights", showLights_);
    settings()->setValue("GeometryWidget/pointsize", pointsize_);

    delete drawable_;
    delete lights_;
    delete glprops_;
}

void GeometryWidget::setGeometry(GEOM::Geometry * g)
{
    drawable_->setGeometry(g);
    update();
}

void GeometryWidget::initGL()
{
    Basic3DWidget::initGL();

    if (!glprops_)
    {
        glprops_ = new GL::Properties;
        glprops_->getProperties();
    }
}

void GeometryWidget::releaseGL()
{
    Basic3DWidget::releaseGL();

    if (drawable_->isReady())
        drawable_->releaseOpenGl();
}

void GeometryWidget::drawGL(const Mat4& projection,
                            const Mat4& cubeViewTrans,
                            const Mat4& viewTrans,
                            const Mat4& trans)
{
    MO_ASSERT(glprops_, "missing initGL");

    using namespace gl;

    MO_CHECK_GL( gl::glClearColor(0.1, 0.2, 0.3, 1.0) );
    MO_CHECK_GL( gl::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    // XXX not working in RM_DIRECT
    MO_CHECK_GL( gl::glEnable(GL_BLEND) );
    MO_CHECK_GL( gl::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

    bool recompile = false;

    if (showTexture_)
    {
        if (!tex_)
        {
            MO_CHECK_GL(glActiveTexture(GL_TEXTURE0));
            tex_ = GL::Texture::createFromImage(
                    QImage(":/texture/default_texture.png"),
                    GL_RGB, GL::ER_IGNORE);
            recompile = true;
        }
    }
    else
    if (tex_)
    {
        tex_->release();
        delete tex_;
        tex_ = 0;
        recompile = true;
    }


    if (showNormalMap_)
    {
        if (!texNorm_)
        {
            QImage img, norm;
            img.load(":/texture/default_texture.png");
            /** @todo nice normalmap in geometrywidget */
            //ImageGenerator::createNormalmap(&norm, &img);
            MO_CHECK_GL(glActiveTexture(GL_TEXTURE0 + 1));
            texNorm_ = GL::Texture::createFromImage(
                        norm, GL_RGB, GL::ER_IGNORE);
            MO_CHECK_GL(glActiveTexture(GL_TEXTURE0));
            recompile = true;
        }
    }
    else
    if (texNorm_)
    {
        texNorm_->release();
        delete texNorm_;
        texNorm_ = 0;
        recompile = true;
    }

    if (showGrid_)
        drawGrid(projection, cubeViewTrans, viewTrans, trans);

    if (recompile && drawable_->isReady())
        drawable_->releaseOpenGl();

    // compile drawable when geometry is ready
    if (!drawable_->isReady() && drawable_->geometry())
    {
        // set source (and flags)
        GL::ShaderSource * src = new GL::ShaderSource();
        src->loadDefaultSource();
        src->addDefine("#define MO_ENABLE_LIGHTING");
        src->addDefine("#define MO_FRAGMENT_LIGHTING");
        src->addDefine(QString("#define MO_NUM_LIGHTS %1").arg(lights_->count()));
        if (tex_)
            src->addDefine("#define MO_ENABLE_TEXTURE");
        if (texNorm_)
            src->addDefine("#define MO_ENABLE_NORMALMAP");
        drawable_->setShaderSource(src);

        // compile
        drawable_->createOpenGl();

        glprops_->setPointSize(pointsize_);

        // bind normal texture slot
        if (texNorm_)
        {
            GL::Uniform * u = drawable_->shader()->getUniform("tex_norm_0");
            if (u)
                u->ints[0] = 1;
            u = drawable_->shader()->getUniform(drawable_->shader()->source()->uniformNameBumpScale());
            if (u)
                u->floats[0] = 1.0;
        }
    }

    if (drawable_->isReady())
    {
        setLights_(showLights_ ? 0.8f : 0.f, glm::inverse(viewTrans));
        drawable_->renderShader(projection, cubeViewTrans, viewTrans, trans, lights_);
    }
}


} // namespace GUI
} // namespace MO
