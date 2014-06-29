/** @file gl.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>

#include "context.h"

namespace MO {
namespace GL {

Context::Context(QObject *parent)
    :   QOpenGLContext(parent)
{
    QOpenGLVersionProfile profile;
    /*profile.setVersion(1, 2);
    glFunctions_ = functions()*/
    //QAbstractOpenGLFunctions
}

Context::~Context()
{

}




} // namespace GL
} // namespace MO
