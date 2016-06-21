/** @file context.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QOpenGLContext>

#include "context.h"
#include "glcontext.h"
#include "glwindow.h"
#include "io/error.h"

namespace MO {
namespace GL {

Context::Context()
    : glContext_    (nullptr)
    , glWindow_     (nullptr)
    , qcontext_     (new QOpenGLContext())
    , surface_      (nullptr)
{
}

Context::Context(GlWindow* win)
    : glContext_    (new GlContext(win))
    , glWindow_     (win)
    , qcontext_     (nullptr)
    , surface_      (nullptr)
{

}

Context::~Context()
{
    delete glContext_;
    delete qcontext_;
}

bool Context::isValid() const
{
    return glContext_->isOk() || qcontext_->isValid();
}

bool Context::makeCurrent()
{
    if (glContext_)
    {
        glContext_->makeCurrent();
        return true;
    }

    if (!surface_)
    {
        MO_GL_WARNING("GL::Context::makeCurrent() without assigned surface");
        return false;
    }

    return qcontext_->makeCurrent(surface_);
}

bool Context::swapBuffers()
{
    if (glWindow_)
        glWindow_->swapBuffers();
    else
        if (qcontext_)
            qcontext_->swapBuffers(surface_);
    return true;
}

} // namespace GL
} // namespace MO
