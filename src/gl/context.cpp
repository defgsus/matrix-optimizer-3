/** @file context.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QOpenGLContext>

#include "context.h"
#include "io/error.h"

namespace MO {
namespace GL {

Context::Context(QObject *parent)
    :   QObject     (parent),
        qcontext_   (new QOpenGLContext(this)),
        surface_    (0)
{
    //QOpenGLVersionProfile profile;
    /*profile.setVersion(1, 2);
    glFunctions_ = functions()*/
    //QAbstractOpenGLFunctions
}

Context::~Context()
{

}

bool Context::isValid() const
{
    return qcontext_->isValid();
}

bool Context::makeCurrent()
{
    if (!surface_)
    {
        MO_GL_WARNING("GL::Context::makeCurrent() without assigned surface");
        return false;
    }

    return qcontext_->makeCurrent(surface_);
}

} // namespace GL
} // namespace MO
