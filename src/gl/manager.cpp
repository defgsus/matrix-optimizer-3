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
                            SLOT(onContextCreated_(uint, MO::GL::Context*)));
        connect(window_, SIGNAL(renderRequest(uint)), SIGNAL(renderRequest(uint)));
    }
    return window_;
}

void Manager::onContextCreated_(uint thread, Context * context)
{
    MO_DEBUG_GL("Manager::onContextCreated_(" << thread << ", " << context << ")");

    emit contextCreated(thread, context);
}

} // namespace GL
} // namespace MO
