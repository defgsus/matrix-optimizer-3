/** @file projectorblender.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#include <QVector>
#include <QImage>
#include <QFileInfo>
#include <QDir>

#include "ProjectorBlender.h"
#include "ProjectionSystemSettings.h"
#include "ProjectorMapper.h"
#include "gl/Shader.h"
#include "gl/ScreenQuad.h"
#include "gl/Texture.h"
#include "gl/FrameBufferObject.h"
#include "gl/OffscreenContext.h"
#include "gl/opengl.h"
#include "io/error.h"

namespace MO {

class ProjectorBlender::Private
{
public:

    ProjectionSystemSettings set;
    QVector<ProjectorMapper> mapper;

    void setSettings(const ProjectionSystemSettings & set);
    GL::Texture * renderBlendTexture(uint index, uint height);
    void exportMaps(const QString &directory);
};


ProjectorBlender::ProjectorBlender(const ProjectionSystemSettings * set)
    : p_    (new Private())
{
    if (set)
        setSettings(*set);
}

ProjectorBlender::~ProjectorBlender()
{
    delete p_;
}

void ProjectorBlender::setSettings(const ProjectionSystemSettings & set)
{
    p_->setSettings(set);
}

GL::Texture * ProjectorBlender::renderBlendTexture(uint index, uint height)
{
    MO_ASSERT(index < p_->set.numProjectors(), "ProjectorBlender::getBlendTexture("
              << index << ") out of range (" << p_->set.numProjectors() << ")");

    return p_->renderBlendTexture(index, height);
}


void ProjectorBlender::Private::setSettings(const ProjectionSystemSettings &settings)
{
    set = settings;

    // create mapper for each projector
    mapper.resize(set.numProjectors());
    for (uint i=0; i<set.numProjectors(); ++i)
        mapper[i].setSettings(set.domeSettings(), set.projectorSettings(i));
}


GL::Texture * ProjectorBlender::Private::renderBlendTexture(uint index, uint height0)
{
    GL::Texture * tex = 0;
    GL::ScreenQuad * quad = 0;
    GL::FrameBufferObject * fbo = 0;

    int width = set.cameraSettings(index).width(),
        height = set.cameraSettings(index).height();

    if (height0 != 0)
    {
        height = height0;
        width = set.cameraSettings(index).aspect() * height0;
    }

    // prepare variables for shader
    std::vector<Mat4>
            projections(set.numProjectors()),
            inverseProjections(set.numProjectors()),
            views(set.numProjectors()),
            inverseViews(set.numProjectors());
    std::vector<Vec2>
            nearFars(set.numProjectors());
    std::vector<Vec4>
            sections(set.numProjectors());
    std::vector<Float>
            aspects(set.numProjectors());

    for (uint i=0; i<set.numProjectors(); ++i)
    {
        projections[i] = mapper[i].getProjecionMatrix();
        inverseProjections[i] = glm::inverse(mapper[i].getProjecionMatrix());
        views[i] = mapper[i].getTransformationMatrix();
        inverseViews[i] = glm::inverse(mapper[i].getTransformationMatrix());

        nearFars[i] = Vec2(mapper[i].nearPlane(), mapper[i].farPlane());
        aspects[i] = mapper[i].aspect();
    }

    try
    {
        // prepare a quad with the blend shader

        quad = new GL::ScreenQuad("ProjectorBlender_quad");

        QString defines =
                QString("#define MO_NUM_PROJECTORS %1\n"
                        "#define MO_BLEND_METHOD %2")
                        .arg(set.numProjectors())
                        .arg(set.blendMethod());

        quad->create(
                    ":/shader/edgeblend.vert",
                    ":/shader/edgeblend.frag",
                    defines);

        // check for existence of uniforms?
        // [blendMethod() -1 means output white,
        //  which we still need to archive somehow,
        //  so we use the same shader as dummy without uniforms]
        bool uniCheck = set.blendMethod() != -1;

        auto u_margin = quad->shader()->getUniform("u_margin", uniCheck);
        auto u_projection = quad->shader()->getUniform("u_projection[0]", uniCheck);
        auto u_inverseProjection = quad->shader()->getUniform("u_inverseProjection[0]", uniCheck);
        auto u_view = quad->shader()->getUniform("u_view[0]", uniCheck);
        auto u_inverseView = quad->shader()->getUniform("u_inverseView[0]", uniCheck);
        auto u_dome_radius = quad->shader()->getUniform("u_dome_radius", uniCheck);
        auto u_nearFar = quad->shader()->getUniform("u_nearFar[0]", uniCheck);
        //auto u_aspect = quad->shader()->getUniform("u_aspect[0]", uniCheck);
        auto u_index = quad->shader()->getUniform("u_index", uniCheck);

        if (uniCheck)
        {
            u_dome_radius->floats[0] = set.domeSettings().radius();
            u_index->ints[0] = index;
            u_margin->floats[0] = set.blendMargin();
            u_margin->floats[1] = set.blendMargin() * mapper[index].aspect();
        }

        // prepare fbo

        fbo = new GL::FrameBufferObject(width, height, gl::GL_RGBA, gl::GL_FLOAT);
        fbo->create();
        fbo->bind();

        // ---- render ----

        MO_CHECK_GL( gl::glViewport(0,0,fbo->width(),fbo->height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        // set uniforms
        quad->shader()->activate();

        if (uniCheck)
        {
            MO_CHECK_GL( gl::glUniformMatrix4fv(
                             u_projection->location(), set.numProjectors(), gl::GL_FALSE,
                             &projections[0][0][0]) );
            MO_CHECK_GL( gl::glUniformMatrix4fv(
                             u_inverseProjection->location(), set.numProjectors(), gl::GL_FALSE,
                             &inverseProjections[0][0][0]) );
            MO_CHECK_GL( gl::glUniformMatrix4fv(
                             u_view->location(), set.numProjectors(), gl::GL_FALSE,
                             &views[0][0][0]) );
            MO_CHECK_GL( gl::glUniformMatrix4fv(
                             u_inverseView->location(), set.numProjectors(), gl::GL_FALSE,
                             &inverseViews[0][0][0]) );
            MO_CHECK_GL( gl::glUniform2fv(
                             u_nearFar->location(), set.numProjectors(),
                             &nearFars[0][0]) );
            //MO_CHECK_GL( gl::glUniform1fv(u_aspect->location(), set.numProjectors(),
            //                                    &aspects[0]) );
        }

        quad->draw(width, height);

        fbo->unbind();

        quad->release();
        delete quad;
        quad = 0;

        // get final texture

        tex = fbo->takeColorTexture();

        fbo->release();
        delete fbo;
        fbo = 0;

        // set texture edge settings
        GL::Texture::setActiveTexture(0);
        tex->bind();
        /** @todo not sure if this will stay until use...
                  [actually i'm never sure. Need to test that once!] */
        tex->setTexParameter(gl::GL_TEXTURE_WRAP_S, gl::GLint(gl::GL_CLAMP_TO_EDGE));
        tex->setTexParameter(gl::GL_TEXTURE_WRAP_T, gl::GLint(gl::GL_CLAMP_TO_EDGE));
    }
    catch (Exception & e)
    {
        if (fbo && fbo->isCreated())
            fbo->release();
        delete fbo;
        if (quad && quad->isCreated())
            quad->release();
        delete quad;
        if (tex && tex->isAllocated())
            tex->release();
        delete tex;

        throw;
    }

    return tex;
}


void ProjectorBlender::exportMaps(const QString &directory)
{
    p_->exportMaps(directory);
}

void ProjectorBlender::Private::exportMaps(const QString &directory)
{
    GL::OffscreenContext ctx;

    ctx.createGl();

    if (!ctx.makeCurrent())
        MO_GL_ERROR("Could not make openGL current");



    for (uint i=0; i<set.numProjectors(); ++i)
    {
        int width = set.cameraSettings(i).width(),
            height = set.cameraSettings(i).height();

        // create framebuffer
        GL::FrameBufferObject fbo(width, height, gl::GL_RGBA, gl::GL_FLOAT);
        fbo.create();
        fbo.bind();

        // init view
        MO_CHECK_GL( gl::glViewport(0,0, width, height) );
        MO_CHECK_GL( gl::glClearColor(0,0,0, 1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );

        // render the texture
        renderBlendTexture(i, 0);

        // export via QImage

        QImage img = fbo.colorTexture()->toQImage();

        QFileInfo fi(QDir(directory), QString("edgemap_%1.png").arg(i));
        if (!img.save(fi.absoluteFilePath()))
            MO_IO_ERROR(WRITE, "Could not save image file\n" << fi.absoluteFilePath());

        fbo.release();
    }
}


} // namespace MO
