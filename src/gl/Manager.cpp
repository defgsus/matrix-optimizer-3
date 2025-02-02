/** @file manager.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <thread>
#include <mutex>

#include <QPixmap>
#include <QImage>
#include <QList>
#include <QPair>

#include "Manager.h"
#include "GlWindow.h"
#include "GlContext.h"
#include "gl/opengl.h"
#include "gl/Texture.h"
#include "gl/TextureRenderer.h"
#include "geom/FreeCamera.h"
#include "SceneRenderer.h"
#include "object/interface/ValueTextureInterface.h"
#include "tool/GeneralImage.h"
#include "object/Scene.h"
#include "object/util/SceneSignals.h"
#include "io/Application.h"
#include "io/time.h"
#include "io/MouseState.h"
#include "io/KeyboardState.h"
#include "io/CurrentTime.h"
#include "io/error.h"
#include "io/log_gl.h"

#if 0
#   include "io/log.h"
#   define MO__D(arg__) MO_PRINT("Manager("<<this<<")::" << arg__)
#else
#   define MO__D(unused__) { }
#endif


namespace MO {
namespace GL {

namespace {

class RenderWindow : public GlWindow
{
public:
    RenderWindow(Manager* mgr, int w, int h)
        : GlWindow  (w, h)
        , manager   (mgr)
    { }

    Manager* manager;
    FreeCamera camera;

protected:

    bool closeEvent() override { return false; }
    void resizeEvent() override
    {
        emit manager->sendResize_(QSize(width(), height()));
        manager->render();
    }

    bool mouseMoveEvent(int x, int y) override
    {
        if (isMouseDown())
        {
            MouseState::globalInstance().setDragPos(
                        QPoint(x, y), QSize(width(), height()));
            manager->render();
        }
        else
            MouseState::globalInstance().setPos(
                        QPoint(x, y), QSize(width(), height()));

        Float fac = 1.f;//e->modifiers() & Qt::SHIFT ?
                    //10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;
        int dx = mouseXDelta(),
            dy = mouseYDelta();

        if (mouseKeys() & MKey_Left)
        {
            camera.moveX(-0.03*fac*dx);
            camera.moveY( 0.03*fac*dy);
        }
        if (mouseKeys() & MKey_Right)
        {
            camera.rotateX(-dy * fac);
            camera.rotateY(-dx * fac);
        }

        return true;
    }

    bool mouseWheelEvent(int delta) override
    {
        Float fac = 1.;//e->modifiers() & Qt::SHIFT ?
                    //10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;
        Float d = std::max(-1, std::min(1, delta ));
        camera.moveZ(-0.3 * d * fac);

        return true;
    }

    bool mouseDownEvent(MouseKeyCode k) override
    {
        MouseState::globalInstance().keyDown(mouseKeyToQt(k));
        MouseState::globalInstance().setDragPos(
                    QPoint(mouseX(), mouseY()), QSize(width(), height()));
        manager->render();
        return true;
    }

    bool mouseUpEvent(MouseKeyCode k) override
    {
        MouseState::globalInstance().keyUp(mouseKeyToQt(k));
        manager->render();
        return true;
    }

    bool keyDownEvent(KeyCode k) override
    {
        KeyboardState::globalInstance().keyDown(k);
        auto e = new QKeyEvent(QKeyEvent::KeyPress,
                               GlWindow::keyToQt(KeyCode(k)),
                               0);
        emit manager->sendKeyPressed_(e);
        manager->render();
        return true;
    }

    bool keyUpEvent(KeyCode k) override
    {
        KeyboardState::globalInstance().keyUp(k);
        manager->render();
        return true;
    }
};


} // namespace




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
        , desiredFps    (60.)
        , messuredFps   (0.)
    { }

    struct ImageRequest
    {
        ValueTextureInterface* iface;
        const GL::Texture* tex;
        QSize res;
        QString id;
        int ifaceChan;
        std::function<void(const QImage&)> callback;
    };

    void startThread();
    void stopThread(bool wait = true);
    void renderLoop();
    QImage renderImage(ImageRequest&);

    Manager* p;

    Scene * scene, *newScene;
    RenderWindow * window;
    SceneRenderer * renderer;
    QList<TextureRenderer*> texRenderers;
    static const int maxTextureRenderers = 3;

    std::function<Double()> timeFunc;

    std::thread* thread;
    volatile bool
        doStop,
        doAnimate,
        doSingleAnimate,
        setNewScene;
    volatile Double
        desiredFps,
        messuredFps;

    std::mutex imageRequestMutex;
    QList<ImageRequest> imageRequests;
};

Manager::Manager(QObject *parent)
    : QObject   (parent)
    , p_        (new Private(this))
{
    MO_DEBUG_GL("Manager::Manager()");

    connect(this, SIGNAL(sendResize_(QSize)),
            this, SIGNAL(outputSizeChanged(QSize)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(sendKeyPressed_(QKeyEvent*)),
            this, SIGNAL(keyPressed(QKeyEvent*)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(sendImage(const GL::Texture*,QString,QImage)),
            this, SIGNAL(imageFinished(const GL::Texture*,QString,QImage)),
            Qt::QueuedConnection);
}

Manager::~Manager()
{
    MO_DEBUG_GL("Manager::~Manager()");

    p_->stopThread(true);

    if (p_->scene)
        p_->scene->releaseRef("Manager: destroy");

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

    if (changed && p_->newScene)
    {
        // connect events from scene to window
        connect(p_->newScene->sceneSignals(), &SceneSignals::renderRequest, [=]()
        {
            //if (p_->scene && p_->timeFunc)
            //    p_->scene->setSceneTime(p_->timeFunc(), false);

            render();
        });
    }

    render();
}

void Manager::setTimeCallback(std::function<Double ()> timeFunc)
{
    p_->timeFunc = timeFunc;
    if (p_->renderer)
        p_->renderer->setTimeCallback(p_->timeFunc);
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
    setCurrentThreadName("GL");

    MO__D("renderLoop()");

    doStop = false;

    MO__D("creating window");
    window = new RenderWindow(p, 320, 320);

    MO__D("creating scene renderer");
    renderer = new SceneRenderer();
    MO__D("creating scene renderer context");
    renderer->createContext(window);
    renderer->setTimeCallback(timeFunc);
    if (!setNewScene && scene)
        renderer->setScene(scene);

    Double prevTime = systemTime(),
           headerUpdateTime = prevTime;

    TimeMessure fpsCount;

    MO__D("Starting render-loop");

    while (!doStop && window->update())
    {
        // render scene, or sleep
        bool doSwap = false;
        if (!doAnimate && !doSingleAnimate)
        {
            sleep_seconds_lowres(1./desiredFps);
        }
        else
        {
            try
            {
                if (setNewScene)
                {
                    MO__D("setting new scene");
                    if (scene)
                    {
                        MO__D("replacing old scene");
                        scene->destroyGlRequest();
                        scene->releaseRef("Manager:release-prev");
                    }
                    renderer->setScene(scene = newScene);
                    if (scene)
                        scene->setManager(p);
                    newScene = nullptr;
                    setNewScene = false;
                }

                if (scene && scene->freeCameraIndex() >= 0)
                    scene->setFreeCameraMatrix(window->camera.getMatrix());

                renderer->setSize(QSize(window->width(), window->height()));
                renderer->render(true);
                doSwap = true;
                doSingleAnimate = false;
            }
            catch (const Exception& e)
            {
                MO_WARNING("EXCEPTION IN OPENGL THREAD: " << e.what());
                sleep_seconds_lowres(1.);
            }
        }

        // fulfill image render requests
        {
            std::lock_guard<std::mutex> lock(imageRequestMutex);
            while (!imageRequests.isEmpty())
            {
                MO__D("image render request '"
                      << imageRequests.front().id << "'");

                auto img = renderImage(imageRequests.front());
                if (!imageRequests.front().id.isEmpty())
                    emit p->sendImage(imageRequests.front().tex,
                                      imageRequests.front().id,
                                      img);
                else if (imageRequests.front().callback)
                    imageRequests.front().callback(img);
                imageRequests.pop_front();
            }
        }


        auto time = systemTime(),
             delta = time - prevTime,
             ddelta = 1. / desiredFps;

        if (delta < ddelta)
        {
            //MO_PRINT("SLEEP " << (ddelta - delta));
            sleep_seconds(ddelta - delta);
        }

        prevTime = systemTime();

        // update window header
        if (time - headerUpdateTime > 1.)
        {
            headerUpdateTime = time;
            if (doAnimate)
                window->setTitle(QString("%1 fps").arg(messuredFps)
                                 .toStdString().c_str());
            else
                window->setTitle("stopped");
        }

        if (doSwap)
        {
            //MO_PRINT("SWAP " << time << " " << delta);
            window->swapBuffers();
        }

        delta = fpsCount.time();
        fpsCount.start();
        if (delta > 0.)
            messuredFps += std::min(1., delta*2.) * (1. / delta - messuredFps);
    }

    if (scene)
    {
        scene->destroyGlNow();
        scene->setGlContext(MO_GFX_THREAD, nullptr);
    }

    for (auto t : texRenderers)
    {
        t->releaseGl();
        delete t;
    }
    texRenderers.clear();

    moCloseGl();

    delete window; window = nullptr;
    delete renderer; renderer = nullptr;
}

QImage Manager::Private::renderImage(ImageRequest& req)
{
    QString errText = tr("error");

    try
    {
        if (!req.tex)
        {
            if (!req.iface)
            {
                return GeneralImage::getErrorImage(
                            tr("nothing\nassigned"), req.res);
            }
            req.tex = req.iface->valueTexture(
                        req.ifaceChan,
                        RenderTime(CurrentTime::time(), MO_GFX_THREAD)
                        );
        }

        if (!req.tex)
            return GeneralImage::getErrorImage(tr("NULL"), req.res);

        if (req.res.isEmpty())
            req.res = QSize(req.tex->width(), req.tex->height());

        // return texture data as-is
        if ((int)req.tex->width() == req.res.width()
         && (int)req.tex->height() == req.res.height())
        {
            req.tex->bind();
            return req.tex->toQImage();
        }

        // find resampler with matching resolution
        GL::TextureRenderer* renderer = nullptr;
        for (auto t : texRenderers)
        if ((int)t->width() == req.res.width()
         && (int)t->height() == req.res.height())
        {
            renderer = t;
            break;
        }

        // none found
        if (!renderer)
        {
            // create one
            if (texRenderers.size() < maxTextureRenderers)
            {
                renderer = new GL::TextureRenderer(req.res.width(), req.res.height());
                texRenderers << renderer;
            }
            else
            {
                // change the one with closest resolution
                int mind = 8000000;
                renderer = texRenderers.front();
                for (auto t : texRenderers)
                {
                    int d = std::abs(t->width()-req.res.width())
                            + std::abs(t->height()-req.res.height());
                    if (d < mind)
                    {
                        renderer = t;
                        mind = d;
                    }
                }

                renderer->setSize(req.res.width(), req.res.height());
            }
        }

        // gl-resize
        renderer->render(req.tex, true);
        // download image
        if (auto stex = renderer->texture())
        {
            stex->bind();
            return stex->toQImage();
        }
    }
    catch (const Exception& e)
    {
        // set error tex
        errText = errText + "\n" + e.what();
    }

    return GeneralImage::getErrorImage(errText, req.res);
}

void Manager::renderImage(const Texture *tex, const QSize &s, const QString& id)
{
    Private::ImageRequest r;
    r.tex = tex;
    r.iface = nullptr;
    r.res = s;
    r.id = id;
    std::lock_guard<std::mutex> lock(p_->imageRequestMutex);
    p_->imageRequests << r;
}

void Manager::renderImage(ValueTextureInterface* iface, int channel,
                          const QSize &s, const QString& id)
{
    Private::ImageRequest r;
    r.iface = iface;
    r.ifaceChan = channel;
    r.tex = nullptr;
    r.res = s;
    r.id = id;
    std::lock_guard<std::mutex> lock(p_->imageRequestMutex);
    p_->imageRequests << r;
}

void Manager::renderImage(ValueTextureInterface* iface, int channel,
                          const QSize &s,
                          ImageCallback foo)
{
    Private::ImageRequest r;
    r.iface = iface;
    r.ifaceChan = channel;
    r.tex = nullptr;
    r.res = s;
    r.callback = foo;
    std::lock_guard<std::mutex> lock(p_->imageRequestMutex);
    p_->imageRequests << r;
}

} // namespace GL
} // namespace MO
