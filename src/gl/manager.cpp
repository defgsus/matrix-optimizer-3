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
#include "io/log.h"

namespace MO {
namespace GL {

Manager::Manager(QObject *parent) :
    QObject(parent),
    scene_      (0),
    window_     (0),
    renderer_   (0)
{
    MO_DEBUG_GL("Manager::Manager()");
}

Manager::~Manager()
{
    MO_DEBUG_GL("Manager::~Manager()");

    if (window_)
        window_->close();
}

Window * Manager::createGlWindow(uint /*thread*/)
{
    if (!window_)
    {
        window_ = new Window();

        connect(window_, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                    this, SLOT(onCameraMatrixChanged_(MO::Mat4)));

        connect(window_, SIGNAL(sizeChanged(QSize)),
                    this, SIGNAL(outputSizeChanged(QSize)));
    }

    if (!renderer_)
    {
        renderer_ = new SceneRenderer();

        if (scene_)
            renderer_->setScene(scene_);

        window_->setRenderer(renderer_);
    }

    return window_;
}

void Manager::setScene(Scene * scene)
{
    bool changed = (scene != scene_);

    scene_ = scene;

    // XXX Would not work if window was not created yet
    if (changed && scene_ && window_)
    {
        // connect events from scene to window
        connect(scene_, SIGNAL(renderRequest()),
                    window_, SLOT(renderLater()));

        connect(scene_, SIGNAL(playbackStarted()),
                    window_, SLOT(startAnimation()));

        connect(scene_, SIGNAL(playbackStopped()),
                    window_, SLOT(stopAnimation()));
    }

    renderer_->setScene(scene);
}

void Manager::onCameraMatrixChanged_(const Mat4 & mat)
{
    emit cameraMatrixChanged(mat);
}

} // namespace GL
} // namespace MO
