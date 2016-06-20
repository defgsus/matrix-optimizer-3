/** @file manager.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include <QThread>

#include "manager.h"
#include "window.h"
#include "context.h"
#include "scenerenderer.h"
#include "object/scene.h"
#include "object/util/scenesignals.h"
#include "io/log_gl.h"
#include "io/error.h"
#include "io/application.h"

namespace MO {
namespace GL {

struct Manager::Private
{
    Private(Manager* p)
        : p         (p)
        , scene     (nullptr)
        , window    (nullptr)
        , renderer  (nullptr)
    { }

    Manager* p;

    Scene * scene;
    Window * window;
    SceneRenderer * renderer;

    std::function<Double()> timeFunc;

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

    if (p_->window)
        p_->window->close();

    delete p_;
}

SceneRenderer * Manager::renderer() const { return p_->renderer; }


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

void Manager::setScene(Scene * scene)
{
    bool changed = (scene != p_->scene);

    p_->scene = scene;

    // XXX Would not work if window was not created yet
    if (changed && p_->scene && p_->window)
    {
        // connect events from scene to window
        connect(p_->scene->sceneSignals(), SIGNAL(renderRequest()),
                    //this, SLOT(onRenderRequest_()));
                    p_->window, SLOT(renderLater()));
    }

    p_->renderer->setScene(scene);
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
    return p_->window && p_->window->isAnimating();
}

void Manager::startAnimate()
{
    if (p_->window)
        p_->window->startAnimation();
}

void Manager::stopAnimate()
{
    if (p_->window)
        p_->window->stopAnimation();
}

} // namespace GL
} // namespace MO
