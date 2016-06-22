/** @file scenerenderer.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#include <QSurface>
#include <QOffscreenSurface>

#include "scenerenderer.h"
#include "context.h"
#include "offscreencontext.h"
#include "object/scene.h"
#include "gl/opengl.h"
#include "io/currenttime.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_gl.h"

#include <QOpenGLContext>
#include "gl/opengl_undef.h"

namespace MO {
namespace GL {


SceneRenderer::SceneRenderer()
    : scene_        (0),
      context_      (0),
      surface_      (0),
      renderSpeed_  (0.)
{

}

SceneRenderer::~SceneRenderer()
{
    delete context_;
}

QSurfaceFormat SceneRenderer::defaultFormat()
{
    QSurfaceFormat format;

    format.setVersion(4, 3);

#ifdef MO_USE_OPENGL_CORE
    format.setProfile(QSurfaceFormat::CoreProfile);
#else
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
#endif

    return format;
}


void SceneRenderer::setScene(Scene *scene)
{
    scene_ = scene;
    lastTime_ = 0;

    if (context_ && scene_)
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
    context_ = new MO::GL::Context();

    context_->setSurface(surface);
    context_->qcontext()->setFormat(surface_->format());

    if (!context_->qcontext()->create())
        MO_GL_ERROR("could not create opengl context");

    moInitGl();

    if (scene_)
        updateSceneGlContext_();

    //emit contextCreated();
}

GL::Context* SceneRenderer::createContext(GlWindow* window)
{
    MO_ASSERT(!context_, "context already created");

    context_ = new MO::GL::Context(window);

    //moInitGl();

    if (scene_)
        updateSceneGlContext_();

    //emit contextCreated();

    MO_DEBUG_GL("SceneRenderer: GlContext ready");

    return context_;
}


OffscreenContext * SceneRenderer::createOffscreenContext()
{
    MO_ASSERT(!context_, "context already created");

    auto ocontext = new OffscreenContext();

    ocontext->createGl();

    if (!ocontext->makeCurrent())
        MO_GL_ERROR("could not make opengl context current");

    moInitGl();

    context_ = ocontext;
    surface_ = ocontext->qsurface();

    if (scene_)
        updateSceneGlContext_();

    //emit contextCreated();

    return ocontext;
}

void SceneRenderer::updateSceneGlContext_()
{
    MO_ASSERT(context_ && scene_, "");

    // XXX here would be a good point to
    // manage different contexts on different threads
    if (scene_->glContext() != context_)
        scene_->setGlContext(MO_GFX_THREAD, context_);
}


void SceneRenderer::render(bool renderToScreen)
{        
    if (!context_ || !scene_ || scene_->isShutDown())
        return;

//    MO_ASSERT(context_, "no context to render");
//    MO_ASSERT(scene_, "no scene to render");

    TimeMessure tm;

    if (!context_->makeCurrent())
        MO_GL_ERROR("could not make context current");

    // update size request
    // XXX This is stupid XXX
    if (context_->size() != size_)
        context_->setSize(size_);

    MO_CHECK_GL( gl::glViewport(0,0, context_->size().width(), context_->size().height()) );
    MO_CHECK_GL( gl::glClearColor(0.1f, 0.1f, 0.1f, 1.0f) );
    MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );

    if (scene_->glContext() != context_)
        scene_->setGlContext(MO_GFX_THREAD, context_);

    // -- get render time --

#ifndef MO_DISABLE_AUDIO
    RenderTime rtime;
    if (timeFuncT_)
    {
        rtime = timeFuncT_();
    }
    else if (timeFuncD_)
    {
        Double time = timeFuncD_();
        //Double time = scene_->sceneTime();
        rtime = RenderTime(
                    time,
                    std::max(0., std::min(1., time - lastTime_)),
                    time * scene_->sampleRate(),
                    scene_->sampleRate(),
                    256 /** @todo have correct buffer size here */,
                    MO_GFX_THREAD);

    }
#else
#error unmaintained
    Double time = scene_->isPlayback()
            ? CurrentTime::time()
            : scene_->sceneTime();
    //scene_->setSceneTime(time);
    // XXX hack
    emit scene_->sceneTimeChanged(time);
#endif

    lastTime_ = rtime.second();

    // -- render --
    try
    {
        scene_->renderScene(rtime, renderToScreen);
    }
    catch (Exception& e)
    {
        MO_CHECK_GL( gl::glBindFramebuffer(gl::GL_FRAMEBUFFER, 0) );
        MO_CHECK_GL( gl::glViewport(0,0, context_->size().width(),
                                         context_->size().height()) );
        MO_CHECK_GL( gl::glClearColor(1,0,0,1) );
        MO_CHECK_GL( gl::glClear(gl::GL_COLOR_BUFFER_BIT) );
        throw e << "\n  in SceneRenderer::render(" << rtime << ")";
    }

    gl::glFlush();
    gl::glFinish();

    //context_->swapBuffers();

    Double fps = tm.time();
    if (fps > 0.)
        fps = 1. / fps;
    renderSpeed_ += 1./10. * (fps - renderSpeed_);

}





} // namespace GL
} // namespace MO
