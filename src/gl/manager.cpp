/** @file manager.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <thread>

#include "manager.h"
#include "glwindow.h"
#include "glcontext.h"
#include "scenerenderer.h"
#include "object/scene.h"
#include "object/util/scenesignals.h"
#include "io/application.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_gl.h"
#include "gl/opengl.h"

namespace MO {
namespace GL {

struct Manager::Private
{
    Private(Manager* p)
        : p             (p)
        , scene         (nullptr)
        , newScene      (nullptr)
        , window        (nullptr)
        , renderer      (nullptr)
        , thread        (nullptr)
        , setNewScene   (false)
    { }

    void startThread();
    void stopThread(bool wait = true);
    void renderLoop();

    Manager* p;

    Scene * scene, *newScene;
    GlWindow * window;
    SceneRenderer * renderer;

    std::function<Double()> timeFunc;

    std::thread* thread;
    volatile bool doStop, setNewScene;
};

Manager::Manager(QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{
    MO_DEBUG_GL("Manager::Manager()");
}

Manager::~Manager()
{
    MO_DEBUG_GL("Manager::~Manager()");

    p_->stopThread(true);

    delete p_;
}

SceneRenderer * Manager::renderer() const { return p_->renderer; }

bool Manager::isWindowVisible() const
{
    return p_->thread != nullptr;
}

void Manager::setWindowVisible(bool e)
{
    if (e)
    {
        if (!p_->thread)
            p_->startThread();
    }
    else
    {
        if (p_->thread)
            p_->stopThread(true);
    }
}

void Manager::render()
{

}

#if 0
Window * Manager::createGlWindow(uint /*thread*/)
{
    if (!p_->window)
    {
        p_->window = new Window();

        connect(p_->window, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                    this, SLOT(onCameraMatrixChanged_(MO::Mat4)));

        connect(p_->window, SIGNAL(sizeChanged(QSize)),
                    this, SIGNAL(outputSizeChanged(QSize)));
    }

    if (!p_->renderer)
    {
        p_->renderer = new SceneRenderer();
        p_->renderer->setTimeCallback(p_->timeFunc);

        if (p_->scene)
            p_->renderer->setScene(p_->scene);

        p_->window->setRenderer(p_->renderer);
    }

    return p_->window;
}
#endif

void Manager::setScene(Scene * scene)
{
    //bool changed = (scene != p_->scene);

    p_->newScene = scene;
    if (p_->newScene)
        p_->newScene->addRef("Manager:setScene");
    p_->setNewScene = true;

    /*
    // XXX Would not work if window was not created yet
    if (changed && p_->scene && p_->window)
    {
        // connect events from scene to window
        connect(p_->scene->sceneSignals(), SIGNAL(renderRequest()),
                    //this, SLOT(onRenderRequest_()));
                    p_->window, SLOT(renderLater()));
    }

    if (p_->renderer)
        p_->renderer->setScene(scene);
    */
}

void Manager::setTimeCallback(std::function<Double ()> timeFunc)
{
    p_->timeFunc = timeFunc;
    if (p_->renderer)
        p_->renderer->setTimeCallback(p_->timeFunc);
}

void Manager::onCameraMatrixChanged_(const Mat4 & mat)
{
    emit cameraMatrixChanged(mat);
}
/*
void Manager::onRenderRequest_()
{
    if (timeFunc_)
        scene_->setSceneTime(timeFunc_(), false);

    window_->renderLater();
}
*/

bool Manager::isAnimating() const
{
    return false;//p_->window && p_->window->isAnimating();
}

void Manager::startAnimate()
{
    //if (p_->window)
    //    p_->window->startAnimation();
}

void Manager::stopAnimate()
{
    //if (p_->window)
    //    p_->window->stopAnimation();
}

void Manager::Private::startThread()
{
    MO_ASSERT(!thread, "duplicate Manager::Private::startThread()");
    thread = new std::thread([=](){ renderLoop(); });
}

void Manager::Private::stopThread(bool wait)
{
    if (!thread || doStop)
        return;
    doStop = true;
    if (wait && thread && thread->joinable())
        thread->join();
    delete thread;
    thread = nullptr;
}

void Manager::Private::renderLoop()
{
    doStop = false;

    window = new GlWindow();

    renderer = new SceneRenderer();
    renderer->createContext(window);
    renderer->setTimeCallback(timeFunc);

    while (!doStop)
    {
        try
        {
            if (setNewScene)
            {
                if (scene)
                {
                    scene->destroyGl();
                    scene->releaseRef("Manager:release-prev");
                }
                renderer->setScene(scene = newScene);
                setNewScene = false;
            }

            renderer->setSize(QSize(window->width(), window->height()));
            renderer->render(true);
        }
        catch (const Exception& e)
        {
            MO_WARNING("EXCEPTION IN OPENGL THREAD: " << e.what());
        }
    }

    if (scene)
    {
        scene->destroyGl();
        scene->releaseRef("Manager:thread-close");
    }

    delete window; window = nullptr;
    delete renderer; renderer = nullptr;
}



} // namespace GL
} // namespace MO
