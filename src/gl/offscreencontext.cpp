/** @file offscreencontext.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.03.2015</p>
*/

#include <QOffscreenSurface>


#include "offscreencontext.h"
#include "scenerenderer.h"
#include "gl/opengl.h"

#include "gl/opengl_undef.h"
#include <QOpenGLContext>
#include "gl/opengl_undef.h"

//#ifdef Q_OS_LINUX
//#   include "X11/Xlib.h"
//#endif

namespace MO {
namespace GL {



OffscreenContext::OffscreenContext(QObject *parent)
    : Context       (parent)
    , p_os_surface_ (0)
{
}

OffscreenContext::~OffscreenContext()
{
    if (p_os_surface_)
    {
        if (p_os_surface_->isValid())
            p_os_surface_->destroy();

        delete p_os_surface_;
    }
}

bool OffscreenContext::createGl()
{
    if (p_os_surface_ && p_os_surface_->isValid())
        p_os_surface_->destroy();

    if (!p_os_surface_)
        p_os_surface_ = new QOffscreenSurface();

    // create QOffscreenSurface
    p_os_surface_->setFormat(SceneRenderer::defaultFormat());
    p_os_surface_->create();
    if (!p_os_surface_->isValid())
        return false;

    // set surface of base Context class
    setSurface(p_os_surface_);
    qcontext()->setFormat(p_os_surface_->format());

//#ifdef Q_OS_LINUX
//    XLockDisplay(???);
//#endif

    bool r = qcontext()->create();

//#ifdef Q_OS_LINUX
//    XUnlockDisplay();
//#endif

    if (!r)
        return false;

    if (!qcontext()->makeCurrent(p_os_surface_))
        return false;

    glbinding::Binding::initialize(glbinding::getCurrentContext());

    return true;
}

bool OffscreenContext::destroyGl()
{
    if (!p_os_surface_)
        return false;

    p_os_surface_->destroy();

    return true;
}

bool OffscreenContext::finish()
{
    gl::GLenum err;

    MO_CHECK_GL_RET_COND(ER_IGNORE, gl::glFlush(), err);
    MO_CHECK_GL_RET_COND(ER_IGNORE, gl::glFinish(), err);
    qcontext()->swapBuffers(qsurface());

    return err == gl::GL_NO_ERROR;
}

} // namespace GL
} // namespace MO
