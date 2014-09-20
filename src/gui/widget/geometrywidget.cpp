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
#include "geom/geometry.h"
#include "img/image.h"
#include "img/imagegenerator.h"

#include "gl/opengl_undef.h"

namespace MO {
namespace GUI {


GeometryWidget::GeometryWidget(RenderMode mode, QWidget *parent) :
    Basic3DWidget   (mode, parent),
    drawable_       (new GL::Drawable("geomwidget")),
    tex_            (0),
    texNorm_        (0),
    lights_         (new GL::LightSettings()),
    showGrid_       (false),
    showTexture_    (false),
    showNormalMap_  (false)
{
    setMinimumSize(128, 128);

    lights_->resize(3);
    lights_->setPosition(0, 1000.f, 2000.f, 800.f);
    lights_->setPosition(1, -2000.f, 1000.f, 1200.f);
    lights_->setPosition(2, 2000.f, -500.f, 1500.f);
    lights_->setColor(0, 1.f, 1.f, 1.f);
    lights_->setColor(1, 0.2f, 0.5f, 1.f);
    lights_->setColor(2, 0.5f, 0.25f, 0.1f);
}

GeometryWidget::~GeometryWidget()
{
    delete drawable_;
    delete lights_;
}

void GeometryWidget::setGeometry(GEOM::Geometry * g)
{
    drawable_->setGeometry(g);
    update();
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
            Image img, norm;
            img.loadImage(":/texture/default_texture.png");
            ImageGenerator::createNormalmap(&norm, &img);
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
        src->addDefine("#define MO_NUM_LIGHTS 3");
        if (tex_)
            src->addDefine("#define MO_ENABLE_TEXTURE");
        if (texNorm_)
            src->addDefine("#define MO_ENABLE_NORMALMAP");
        drawable_->setShaderSource(src);

        // compile
        drawable_->createOpenGl();

        // bind normal texture slot
        if (texNorm_)
        {
            GL::Uniform * u = drawable_->shader()->getUniform("tex_norm_0");
            if (u)
                u->ints[0] = 1;
        }
    }

    if (drawable_->isReady())
        drawable_->renderShader(projection, cubeViewTrans, viewTrans, trans, lights_);
}


} // namespace GUI
} // namespace MO
