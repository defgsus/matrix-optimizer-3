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
#include "io/log.h"

namespace MO {
namespace GL {

Manager::Manager(QObject *parent) :
    QObject(parent),
    window_ (0)
{
    MO_DEBUG_GL("Manager::Manager()");
}

Manager::~Manager()
{
    MO_DEBUG_GL("Manager::~Manager()");

    if (window_)
        window_->close();
}

Window * Manager::createGlWindow(uint thread)
{
    if (!window_)
    {
        window_ = new Window();
        //QThread * thrd = new QThread(this);
        //thrd->start();
        //window_->moveToThread(thrd);
        window_->setThread(thread);
        connect(window_, SIGNAL(contextCreated(uint, MO::GL::Context*)),
                    this, SLOT(onContextCreated_(uint, MO::GL::Context*)));
        connect(window_, SIGNAL(renderRequest(uint)),
                    this, SIGNAL(renderRequest(uint)));
        connect(window_, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                    this, SLOT(onCameraMatrixChanged_(MO::Mat4)));
        connect(window_, SIGNAL(sizeChanged(QSize)),
                    this, SIGNAL(outputSizeChanged(QSize)));
    }
    return window_;
}

void Manager::onContextCreated_(uint thread, Context * context)
{
    MO_DEBUG_GL("Manager::onContextCreated_(" << thread << ", " << context << ")");

    emit contextCreated(thread, context);
}

void Manager::onCameraMatrixChanged_(const Mat4 & mat)
{
    emit cameraMatrixChanged(mat);
}

} // namespace GL
} // namespace MO
