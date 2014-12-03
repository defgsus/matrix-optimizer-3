/** @file scenerenderer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#include <QSurface>

#include "scenerenderer.h"
#include "context.h"
#include "object/scene.h"
#include "io/error.h"
#include "gl/opengl.h"
#include "io/currenttime.h"

#include <QOpenGLContext>
#include "gl/opengl_undef.h"

namespace MO {
namespace GL {


SceneRenderer::SceneRenderer(QObject *parent)
    : QObject       (parent),
      scene_        (0),
      context_      (0),
      surface_      (0)
{

}

SceneRenderer::~SceneRenderer()
{
    // XXX
    //delete context_;
}

QSurfaceFormat SceneRenderer::defaultFormat()
{
    QSurfaceFormat format;

    format.setVersion(3, 3);

#ifdef __APPLE__
    format.setProfile(QSurfaceFormat::CoreProfile);
#else
    //format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setProfile(QSurfaceFormat::CoreProfile);
#endif

    return format;
}


void SceneRenderer::setScene(Scene *scene)
{
    scene_ = scene;

    if (context_)
        updateSceneGlContext_();
}

void SceneRenderer::setSize(const QSize &resolution)
{
    size_ = resolution;

    if (context_)
        context_->setSize(resolution);
}


void SceneRenderer::createContext(QSurface * surface)
{
    MO_ASSERT(!context_, "context already created");

    surface_ = surface;
    context_ = new MO::GL::Context(this);

    context_->setSurface(surface);
    context_->qcontext()->setFormat(surface_->format());

    if (!context_->qcontext()->create())
        MO_GL_ERROR("could not create opengl context");

    if (scene_)
        updateSceneGlContext_();

    emit contextCreated();
}

void SceneRenderer::updateSceneGlContext_()
{
    MO_ASSERT(context_ && scene_, "");

    // XXX here would be a good point to
    // manage different contexts on different threads
    if (scene_->glContext() != context_)
        scene_->setGlContext(MO_GFX_THREAD, context_);
}


void SceneRenderer::render()
{
    if (!context_ || !scene_ || scene_->isShutDown())
        return;

//    MO_ASSERT(context_, "no context to render");
//    MO_ASSERT(scene_, "no scene to render");

    if (!context_->qcontext()->makeCurrent(surface_))
        MO_GL_ERROR("could not make context current");

    // update size request
    if (context_->size() != size_)
        context_->setSize(size_);

    moInitGl();

    MO_CHECK_GL( gl::glViewport(0,0, context_->size().width(), context_->size().height()) );
    MO_CHECK_GL( gl::glClearColor(0.1f, 0.1f, 0.1f, 1.0f) );
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );

    if (scene_->glContext() != context_)
        scene_->setGlContext(MO_GFX_THREAD, context_);

#ifndef MO_DISABLE_AUDIO
    Double time = timeFunc_ ? timeFunc_() : 0.0;
    //Double time = scene_->sceneTime();
#else
    Double time = scene_->isPlayback()
            ? CurrentTime::time()
            : scene_->sceneTime();
    //scene_->setSceneTime(time);
    // XXX hack
    emit scene_->sceneTimeChanged(time);
#endif

    scene_->renderScene(time, MO_GFX_THREAD);

    context_->qcontext()->swapBuffers(surface_);
}





} // namespace GL
} // namespace MO
