
#ifdef MO_OS_UNIX

#include "gl/glcontext_private.h"
#include "gl/x11/glwindow_x11.h"
#include "gl/glwindow.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_gl.h"

namespace MO {
namespace GL {


void GlContext::makeCurrent()
{
    if (!p_->isValid)
        return;

    bool r = glXMakeContextCurrent( p_->info.display, p_->info.glxwin,
                                    p_->info.glxwin, p_->info.context );
    if (!r)
        MO_GL_ERROR("GlContext:: error on glXMakeContextCurrent(...)");
}

void GlContext::makeCurrent(GlWindow* win)
{
    if (!p_->isValid || !win || !win->isOk())
        return;

    if (win == p_->window)
    {
        makeCurrent();
        return;
    }

    // assign window to this context
    p_->info.xwin = win->info().win;
    p_->info.glxwin = win->info().glxwin;

    bool r = glXMakeContextCurrent( p_->info.display, p_->info.glxwin,
                                    p_->info.glxwin, p_->info.context );
    if (!r)
        MO_GL_ERROR("GlContext:: error on glXMakeContextCurrent(...)");
}


void GlContext::create(GlWindow *win, int majorV, int minorV)
{
    if (!win->isOk())
        return;

    // release any existing context
    release();

    // store locally
    p_->window = win;


    // copy techinfo from window to local
    GlWindowTech wininfo = win->info();

    // copy stuff
    p_->info.display = wininfo.display;
    p_->info.screen = wininfo.screen;
    p_->info.xwin = wininfo.win;

#if 1
    // get glXCreateContextAttribs
    p_->info.procCreateContextAttribs = (PFNGLXCREATECONTEXTATTRIBSARBPROC)
            glXGetProcAddress((GLubyte*)("glXCreateContextAttribsARB"));
    if (!p_->info.procCreateContextAttribs)
    {
        p_->isValid = false;
        MO_GL_ERROR("Could not load glXCreateContextAttribgsARB function");
    }

    int context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, majorV,
        GLX_CONTEXT_MINOR_VERSION_ARB, minorV,
        GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                                     //GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0L
    };

    // create a versioned context
    p_->info.context = p_->info.procCreateContextAttribs(
                p_->info.display, wininfo.fbconfig[0],
                // share-context
                nullptr,
                // direct
                True,
                context_attribs);

#else
    // create a glx context
    p_->info.context = glXCreateNewContext(
                p_->info.display, wininfo.fbconfig[0], GLX_RGBA_TYPE, NULL, True);
#endif

    XSync(p_->info.display, False);

    // create a GLXwindow from the Xwindow
    p_->info.glxwin = glXCreateWindow(
                p_->info.display, wininfo.fbconfig[0], p_->info.xwin, NULL );
    win->info().glxwin = p_->info.glxwin;

    p_->isValid = true;

    makeCurrent();

    // --- load opengl functions

    moInitGl();

    // --- load glx extensions

    p_->info.procSwap = (PFNGLXSWAPINTERVALEXTPROC)
            glXGetProcAddress((GLubyte*)("glXSwapIntervalEXT"));
    if (!p_->info.procSwap)
    {
        p_->isValid = false;
        MO_GL_ERROR("Could not load glXSwapIntervalEXT function");
    }

    setSyncMode(p_->syncMode);
    setViewport();
}



void GlContext::release()
{
    if (!p_->isValid)
        return;

    MO_DEBUG_GL("Destroying context");

    // release glx context
    glXMakeContextCurrent( p_->info.display, p_->info.glxwin,
                           p_->info.glxwin, p_->info.context );

    glXDestroyContext(p_->info.display, p_->info.context);
    p_->info.context = 0;

    // get rid of glx window
    glXDestroyWindow(p_->info.display, p_->info.glxwin);
    p_->info.glxwin = 0;

    p_->isValid = false;

}

/** @todo x11 GlContext::setSync does not seem to work */
void GlContext::setSyncMode(Sync mode)
{
    p_->syncMode = mode;

    p_->info.procSwap(p_->info.display, p_->info.glxwin, p_->syncMode);
}

void GlContext::setViewport()
{
    if (!p_->isValid
            || !p_->window)
        return;

    MO_CHECK_GL_THROW( gl::glViewport(0, 0,
                                      p_->window->width(), p_->window->height() ) );
}

} // namespace GL
} // namespace MO

#endif // MO_OS_UNIX
