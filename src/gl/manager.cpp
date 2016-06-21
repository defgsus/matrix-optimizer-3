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
#include "gl/opengl.h"
#include "scenerenderer.h"
#include "object/scene.h"
#include "object/util/scenesignals.h"
#include "io/application.h"
#include "io/time.h"
#include "io/mousestate.h"
#include "io/keyboardstate.h"
#include "io/error.h"
#include "io/log_gl.h"


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
        , doStop        (false)
        , doAnimate     (false)
        , doSingleAnimate(false)
        , setNewScene   (false)
        , messuredFps   (0.)
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
    volatile bool
        doStop,
        doAnimate,
        doSingleAnimate,
        setNewScene;
    volatile Double messuredFps;
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
        render();
    }
    else
    {
        if (p_->thread)
            p_->stopThread(true);
    }
}

void Manager::render() { p_->doSingleAnimate = true; }

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
    bool changed = (scene != p_->scene && scene != p_->newScene);

    p_->newScene = scene;
    if (p_->newScene)
        p_->newScene->addRef("Manager:setScene");
    p_->setNewScene = true;

    // XXX Would not work if window was not created yet
    if (changed && p_->newScene)
    {
        // connect events from scene to window
        connect(p_->newScene->sceneSignals(), &SceneSignals::renderRequest, [=]()
        {
            if (p_->scene && p_->timeFunc)
                p_->scene->setSceneTime(p_->timeFunc(), false);

            render();
        });
    }
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

bool Manager::isAnimating() const
{
    return p_->doAnimate;
}

Double Manager::messuredFps() const
{
    return p_->messuredFps;
}

void Manager::startAnimate() { p_->doAnimate = true; }
void Manager::stopAnimate() { p_->doAnimate = false; }

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

    Double prevTime = systemTime(),
           headerUpdateTime = prevTime;

    TimeMessure fpsCount;

    while (!doStop && window->update())
    {
        if (!doAnimate && !doSingleAnimate)
        {
            sleep_seconds_lowres(.1);
        }
        else
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
                    newScene = nullptr;
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

        auto time = systemTime(),
             delta = time - prevTime,
             ddelta = 1. / 60.;

        if (delta < ddelta)
        {
            //MO_PRINT("SLEEP " << (ddelta - delta));
            sleep_seconds(ddelta - delta);
        }

        prevTime = systemTime();

        if (time - headerUpdateTime > 1.)
        {
            headerUpdateTime = time;
            window->setTitle(QString("%1 fps").arg(messuredFps).toStdString().c_str());
        }

        if (doAnimate || doSingleAnimate)
        {
            //MO_PRINT("SWAP " << time << " " << delta);
            window->swapBuffers();
        }
        doSingleAnimate = false;

        delta = fpsCount.time();
        fpsCount.start();
        if (delta > 0.)
            messuredFps += std::min(1., delta*2.) * (1. / delta - messuredFps);
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
