/** @file gl.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#include <QOpenGLFramebufferObject>
#include "context.h"

namespace MO {
namespace GL {

Context::Context(QObject *parent)
    :   QOpenGLContext(parent)
{

}

Context::~Context()
{

}




} // namespace GL
} // namespace MO
