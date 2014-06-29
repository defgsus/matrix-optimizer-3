/** @file manager.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

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

Window * Manager::createGlWindow()
{
    if (!window_)
    {
        window_ = new Window();
        connect(window_, SIGNAL(contextCreated(MO::GL::Context*)),
                            SLOT(onContextCreated_(MO::GL::Context*)));
        connect(window_, SIGNAL(renderRequest()), SIGNAL(renderRequest()));
    }
    return window_;
}

void Manager::onContextCreated_(Context * context)
{
    MO_DEBUG_GL("Manager::onContextCreated_(" << context << ")");

    emit contextCreated(context);
}

} // namespace GL
} // namespace MO
