/** @file projectorblender.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.10.2014</p>
*/

#include <QVector>


#include "projectorblender.h"
#include "projectionsystemsettings.h"
#include "projectormapper.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/screenquad.h"
#include "gl/texture.h"
#include "gl/framebufferobject.h"
#include "io/error.h"

namespace MO {

class ProjectorBlender::Private
{
public:

    ProjectionSystemSettings set;
    QVector<ProjectorMapper> mapper;

    void setSettings(const ProjectionSystemSettings & set);
    GL::Texture * renderBlendTexture(uint index);
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

GL::Texture * ProjectorBlender::renderBlendTexture(uint index)
{
    MO_ASSERT(index < p_->set.numProjectors(), "ProjectorBlender::getBlendTexture("
              << index << ") out of range (" << p_->set.numProjectors() << ")");

    return p_->renderBlendTexture(index);
}


void ProjectorBlender::Private::setSettings(const ProjectionSystemSettings &settings)
{
    set = settings;

    // create mapper for each projector
    mapper.resize(set.numProjectors());
    for (uint i=0; i<set.numProjectors(); ++i)
        mapper[i].setSettings(set.domeSettings(), set.projectorSettings(i));
}


GL::Texture * ProjectorBlender::Private::renderBlendTexture(uint index)
{
    GL::Texture * tex = 0;
    GL::ScreenQuad * quad = 0;
    GL::FrameBufferObject * fbo = 0;

    const int width = 320,
              height = 180;

    // prepare variables for shader
    std::vector<Mat4>
            projections(set.numProjectors()),
            inverseProjections(set.numProjectors()),
            views(set.numProjectors()),
            inverseViews(set.numProjectors());
    std::vector<Vec2>
            nearFars(set.numProjectors());
    for (uint i=0; i<set.numProjectors(); ++i)
    {
        projections[i] = mapper[i].getProjecionMatrix();
        inverseProjections[i] = glm::inverse(mapper[i].getProjecionMatrix());
        views[i] = mapper[i].getTransformationMatrix();
        inverseViews[i] = glm::inverse(mapper[i].getTransformationMatrix());

        nearFars[i] = Vec2(mapper[i].nearPlane(), mapper[i].farPlane());
    }

    try
    {
        // prepare a quad with the blend shader

        quad = new GL::ScreenQuad("ProjectorBlender_quad");

        quad->create(
                    ":/shader/edgeblend.vert",
                    ":/shader/edgeblend.frag",
                    QString("#define MO_NUM_PROJECTORS %1")
                        .arg(set.numProjectors()));

        auto u_projection = quad->shader()->getUniform("u_projection[0]", true);
        auto u_inverseProjection = quad->shader()->getUniform("u_inverseProjection[0]", true);
        auto u_view = quad->shader()->getUniform("u_view[0]", true);
        auto u_inverseView = quad->shader()->getUniform("u_inverseView[0]", true);
        auto u_dome_radius = quad->shader()->getUniform("u_dome_radius", true);
        auto u_nearFar = quad->shader()->getUniform("u_nearFar[0]", true);
        auto u_index = quad->shader()->getUniform("u_index", true);

        u_dome_radius->floats[0] = set.domeSettings().radius();
        u_index->ints[0] = index;

        // prepare fbo

        fbo = new GL::FrameBufferObject(width, height, gl::GL_RGBA, gl::GL_FLOAT);
        fbo->create();
        fbo->bind();

        // render

        MO_CHECK_GL( gl::glViewport(0,0,fbo->width(),fbo->height()) );
        MO_CHECK_GL( gl::glClearColor(1,1,1,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        MO_CHECK_GL( gl::glDisable(gl::GL_DEPTH_TEST) );
        MO_CHECK_GL( gl::glEnable(gl::GL_BLEND) );
        MO_CHECK_GL( gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA) );

        quad->shader()->activate();

        MO_CHECK_GL( gl::glUniformMatrix4fv(u_projection->location(), set.numProjectors(), gl::GL_FALSE,
                                            &projections[0][0][0]) );
        MO_CHECK_GL( gl::glUniformMatrix4fv(u_inverseProjection->location(), set.numProjectors(), gl::GL_FALSE,
                                            &inverseProjections[0][0][0]) );
        MO_CHECK_GL( gl::glUniformMatrix4fv(u_view->location(), set.numProjectors(), gl::GL_FALSE,
                                            &views[0][0][0]) );
        MO_CHECK_GL( gl::glUniformMatrix4fv(u_inverseView->location(), set.numProjectors(), gl::GL_FALSE,
                                            &inverseViews[0][0][0]) );
        MO_CHECK_GL( gl::glUniform2fv(u_nearFar->location(), set.numProjectors(),
                                            &nearFars[0][0]) );

        quad->draw(width, height);

        fbo->unbind();

        quad->release();
        delete quad;
        quad = 0;

        // get final texture

        tex = fbo->takeColorTexture();

        fbo->release();
    }
    catch (Exception & e)
    {
        delete fbo;
        delete quad;
        delete tex;

        throw;
    }

    return tex;
}


} // namespace MO
