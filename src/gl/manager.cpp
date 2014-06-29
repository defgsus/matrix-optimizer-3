/** @file manager.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "manager.h"
#include "window.h"
#include "context.h"

namespace MO {
namespace GL {

Manager::Manager(QObject *parent) :
    QObject(parent)
{
}


Window * Manager::createGlWindow()
{
    auto c = new Context(this);
    auto w = new Window();
    w->setContext(c);

    return w;
}



} // namespace GL
} // namespace MO
