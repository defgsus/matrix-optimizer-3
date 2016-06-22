
#ifdef MO_OS_WIN

#include "gl/glcontext_private.h"
#include "gl/glwindow.h"
#include "gl/win32/glwindow_win32.h"
#include "gl/win32/wglext.h"
#include "gl/win32/winerror.h"
#include "gl/opengl.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_gl.h"

#ifdef _MSC_VER
// argument conversion, possible loss of data
#pragma warning(disable : 4267)
#endif

namespace MO {
namespace GL {


void GlContext::makeCurrent()
{
    if (!p_->isValid)
        return;

    if (!wglMakeCurrent(p_->info.hdc, p_->info.hrc))
        MO_GL_ERROR("Could not make wgl context current '"
                      << getLastWinErrorString() << "'");
}

void GlContext::makeCurrent(GlWindow * win)
{
    if (!p_->isValid || !win || !win->isOk())
        return;

    if (win == p_->window)
    {
        makeCurrent();
        return;
    }

    // assign window to this context
    p_->window = win;
    p_->info.hwnd = win->info().hwnd;
    p_->info.hdc = win->info().deviceContext;

    if (p_->info.assignedHdc.find(p_->info.hdc)
            == p_->info.assignedHdc.end())
    {
        // set pixel format for dc
        if (!SetPixelFormat(p_->info.hdc, p_->info.pfdnr, &p_->info.pfd))
        {
            MO_GL_ERROR("Could not set pixel format: '" << getLastWinErrorString() << "'");
        }
        p_->info.assignedHdc.insert(p_->info.hdc);
    }

    if (!wglMakeCurrent(p_->info.hdc, p_->info.hrc))
        MO_GL_ERROR("Could not make wgl context current to window '"
                      << win->title() << "': '" << getLastWinErrorString() << "'");
}

#if 0
void GlContext::swapBuffers()
{
    p_->swapBuffers(p_->info.hdc);
}

void GlContext::swapBuffers(Window* win)
{
    p_->swapBuffers(win->info().deviceContext);
}

void GlContext::Private::swapBuffers(HDC hdc)
{
    if (!isValid)
        return;

    // send everything over
    //CHECK_GL( glFlush() );
    // wait for gpu processing
    //CHECK_GL( glFinish() );

    if (!SwapBuffers(hdc))
        MO_GL_ERROR("Error on SwapBuffers(): '" << getLastWinErrorString() << "'");

    // count fps

    double ti = systemTime();

    if (lastFlushTime > 0. && ti > lastFlushTime)
    {
        framesPerSecond = 1. / (ti - lastFlushTime);
#ifdef MO_ENABLE_FPS_TRACE
        fpsTrace->addValue(framesPerSecond);
#endif
    }
    lastFlushTime = ti;
}
#endif

namespace
{
    /** Loads the wgl extensions, once a context is loaded */
    void getWglFunctions(GlContextTech& info)
    {
    #ifdef KATJA_WAS_HERE
    --------------------------------------------------------------------------------
       SCHLEIFE   SCHLEIFE   SCHLEIFE   SCHLEIFE   SCHLEIFE   SCHLEIFE   SCHLEIFE
    --------------------------------------------------------------------------------
    #endif

        info.wglExt = new WglExt;
        memset(info.wglExt, 0, sizeof(WglExt));

    #define WGL__GETFUNC(name__, require__) \
        info.wglExt->name__ = (WglExt::PFN##name__)wglGetProcAddress(#name__); \
        if (require__ && !info.wglExt->name__) \
            MO_GL_ERROR(#name__ " is not supported");

        WGL__GETFUNC(wglCreateContextAttribsARB, true);
        WGL__GETFUNC(wglSwapIntervalEXT, false);

        WGL__GETFUNC(wglJoinSwapGroupNV, false);
        WGL__GETFUNC(wglBindSwapBarrierNV, false);
        WGL__GETFUNC(wglQuerySwapGroupNV, false);
        WGL__GETFUNC(wglQueryMaxSwapGroupsNV, false);
        WGL__GETFUNC(wglQueryFrameCountNV, false);
        WGL__GETFUNC(wglResetFrameCountNV, false);

    #undef WGL__GETFUNC
    }

    HGLRC createVersionedContext(const GlContextTech& info, int major, int minor)
    {
        // max 8 attributes plus terminator
        int attribs[9] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, major,
            WGL_CONTEXT_MINOR_VERSION_ARB, minor,
            0
        };

        MO_DEBUG_GL("GlContext: wglCreateContextAttribsARB()");
        auto context = info.wglExt->wglCreateContextAttribsARB(info.hdc, 0, attribs);
        if (!context)
            MO_GL_ERROR("Could not create extended wgl OpenGL "
                          << major << "." << minor << " context");
        // delete non-unneeded older context
        wglDeleteContext(info.hrc);

        return context;
    }

} // namespace

void GlContext::create(GlWindow *win, int major, int minor)
{
    MO_DEBUG_GL("Creating OpenGL Context " << major << "." << minor);

    if (!win->isOk())
        MO_GL_ERROR("Can not create context for invalid window");

    // release any existing context
    release();

    // store locally
    p_->window = win;

    // get window's device context handle
    p_->info.hwnd = win->info().hwnd;
    p_->info.hdc = GetDC(p_->info.hwnd);

    // check
    if (p_->info.hdc == 0)
    {
        MO_GL_ERROR("Could not get device context from window");
    }

    // make pixelformat structure
    memset(&p_->info.pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    p_->info.pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    p_->info.pfd.nVersion = 1;
    p_->info.pfd.dwFlags =
        PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    // frame buffer type
    p_->info.pfd.iPixelType = PFD_TYPE_RGBA;
    p_->info.pfd.cColorBits = 32;
    p_->info.pfd.cDepthBits = 24;
    p_->info.pfd.cStencilBits = 8;
    p_->info.pfd.iLayerType = PFD_MAIN_PLANE;

    MO_DEBUG_GL("GlContext: ChoosePixelFormat()");

    // find closest match
    p_->info.pfdnr = ChoosePixelFormat(p_->info.hdc, &p_->info.pfd);

    // check
    if (p_->info.pfdnr == 0)
    {
        MO_GL_ERROR("Could not find pixel format for descriptor");
    }

    // set pixel format for dc
    MO_DEBUG_GL("GlContext: SetPixelFormat()");
    if (!SetPixelFormat(p_->info.hdc, p_->info.pfdnr, &p_->info.pfd))
    {
        MO_GL_ERROR("Could not set pixel format: '" << getLastWinErrorString() << "'");
    }

    // context
    MO_DEBUG_GL("GlContext: wglCreateContext()");
    p_->info.hrc = wglCreateContext(p_->info.hdc);

    // check
    if (!p_->info.hrc)
        MO_GL_ERROR("Could not create wgl context");

    // make current
    if (!wglMakeCurrent(p_->info.hdc, p_->info.hrc))
        MO_GL_ERROR("Could not make wgl context current");

    // get wgl extensions
    getWglFunctions(p_->info);

    if (major > 1 || minor > 1)
        p_->info.hrc = createVersionedContext(p_->info, major, minor);

    MO_DEBUG_GL("GlContext: created..");

    p_->isValid = true;

    makeCurrent();

    moInitGl();

    setSyncMode(p_->syncMode);

    MO_DEBUG_GL("GlContext: creation finished");
}



void GlContext::release()
{
    if (!p_->isValid)
        return;

    try
    {
        if (!wglMakeCurrent(p_->info.hdc, p_->info.hrc))
            MO_GL_ERROR("Could not make context current: "
                        << getLastWinErrorString());

        if (!ReleaseDC(p_->info.hwnd, p_->info.hdc))
            MO_GL_ERROR("Could not release device context: "
                        << getLastWinErrorString());

        if (!wglDeleteContext(p_->info.hrc))
            MO_GL_ERROR("Could not delete context: "
                        << getLastWinErrorString());
    }
    catch (const Exception& e)
    {
        MO_WARNING(e.what());
    }

    p_->info.hdc = 0;
    p_->info.hwnd = 0;
    p_->info.hrc = 0;
    p_->isValid = false;

    delete p_->info.wglExt; p_->info.wglExt = 0;
}

void GlContext::setSyncMode(Sync mode)
{
    MO_DEBUG_GL("GlContext::setSyncMode(" << mode << ")");

    p_->syncMode = mode;

    if (p_->isValid && p_->info.wglExt->wglSwapIntervalEXT)
        p_->info.wglExt->wglSwapIntervalEXT(mode);
}

void GlContext::setViewport()
{
    MO_CHECK_GL_THROW( gl::glViewport(0, 0, width(), height()) );
}

#if 0
unsigned GlContext::getMaxSwapGroups() const
{
    if (!p_->info.wglExt->wglQueryMaxSwapGroupsNV)
        return 0;

    gl::GLuint groups, barriers;
    if (!p_->info.wglExt->wglQueryMaxSwapGroupsNV(p_->info.hdc, &groups, &barriers))
        return 0;

    return groups;
}

void GlContext::joinSwapGroup(unsigned g)
{
    // http://oss.sgi.com/projects/performer/mail/info-performer/perf-06-01/0049.html

    if (!p_->info.wglExt->wglJoinSwapGroupNV
     || !p_->info.wglExt->wglBindSwapBarrierNV)
    {
        MO_ERROR("JoinSwapGroup not supported");
        return;
    }

    if (!p_->info.wglExt->wglJoinSwapGroupNV(p_->info.hdc, g))
        MO_ERROR("JoinSwapGroup failed");
    if (!p_->info.wglExt->wglBindSwapBarrierNV(g, 1))
        MO_ERROR("BindSwapBarrier failed");
}

void GlContext::swapBuffers(const std::vector<Context*>& contexts)
{
    std::vector<WGLSWAP> swaps(contexts.size());
    for (size_t i=0; i<contexts.size(); ++i)
    {
        swaps[i].hdc = contexts[i]->p_->info.hdc;
        swaps[i].uiFlags = 0;
    }

    MO_DEBUG_GL("SwapMultipleBuffers(" << swaps.size() << ", " << &swaps[0] << ")");
    wglSwapMultipleBuffers(swaps.size(), &swaps[0]);
}
#endif


} // namespace GL
} // namespace MO

#endif // MO_OS_WIN
