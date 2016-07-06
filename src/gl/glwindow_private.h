/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/20/2016</p>
*/

#ifndef MOSRC_GL_GLWINDOW_PRIVATE_H
#define MOSRC_GL_GLWINDOW_PRIVATE_H

#include "io/architecture.h"

#ifdef MO_OS_WIN
#   include "win32/glwindow_win32.h"


// check the types of the GlWindow friend processEvent_()
#ifdef MO_OS_64BIT
    static_assert(sizeof(LRESULT) == sizeof(int64_t), "type mismatch");
    static_assert(sizeof(UINT) == sizeof(unsigned int), "type mismatch");
    static_assert(sizeof(WPARAM) == sizeof(uint64_t), "type mismatch");
    static_assert(sizeof(LPARAM) == sizeof(int64_t), "type mismatch");
#elif defined(MO_OS_32BIT)
    static_assert(sizeof(LRESULT) == sizeof(int32_t), "type mismatch");
    static_assert(sizeof(UINT) == sizeof(unsigned int), "type mismatch");
    static_assert(sizeof(WPARAM) == sizeof(uint32_t), "type mismatch");
    static_assert(sizeof(LPARAM) == sizeof(int32_t), "type mismatch");
#endif

#elif MO_OS_UNIX
#   include "x11/glwindow_x11.h"
#else
#   error undefined OS
#endif

#include "glwindow.h"
#ifdef MO_ENABLE_FPS_TRACE
#   include "tool/valuebuffer.h"
#endif

namespace MO {
namespace GL {

struct GlWindow::PrivateW
{
    PrivateW(GlWindow * p)
        : pwin      (p)
        , curX      (0)
        , curY      (0)
        , curWidth  (0)
        , curHeight (0)
        , oldX      (0)
        , oldY      (0)
        , oldWidth  (0)
        , oldHeight (0)
        , mouseX    (0)
        , mouseY    (0)
        , mouseKeys (0)
        , parent    (0)
        , isValid   (false)
        , isDestroyed(false)
        , isOnTop   (false)
        , isMouseOver(false)
        , isFocus   (false)
        , isFullscreen(false)
        , isCloseable(true)
        , lastFlushTime     (0.)
        , framesPerSecond   (0.)
        , framesRendered    (0)
#ifdef MO_ENABLE_FPS_TRACE
        , fpsTrace          (new ValueBuffer())
#endif
    { }

    ~PrivateW()
    {
#ifdef MO_ENABLE_FPS_TRACE
        delete fpsTrace;
#endif
    }

    /** init a window context.
        @throws Exception */
    void createWindow();

    /** free all the references. */
    void destroyWindow();

    /** called whenever the window got resized,
        it updates the x,y,width and height fields */
    void onResize();

    /** Convenience function to set mouseKeys state */
    void setMouseKey(MouseKeyCode k, bool down)
    {
        if (!down)
            mouseKeys &= ~int(k);
        else
            mouseKeys |= int(k);
    }

    /** Link to class */
    GlWindow * pwin;

    int curX, curY, curWidth, curHeight,
        /** @{ used for restoring size after fullscreen */
        oldX, oldY, oldWidth, oldHeight,
        /** @} */
        /** @{ Current mouse position */
        mouseX, mouseY, /** @} */
        /** @{ Current mouse delta between last and this event */
        mouseXDelta, mouseYDelta; /** @} */
    /** Current pressed mouse keys */
    MouseKeyCodes mouseKeys;

    /** maybe has parent window, or null */
    GlWindow * parent;

    bool isValid,
        /** this will be true before destruction of window */
        isDestroyed,
        /** stay always on top */
        isOnTop,
        /** is mouse over client area? */
        isMouseOver,
        /** when window has key focus */
        isFocus,
        /** true when in fullscreen mode */
        isFullscreen,
        isCloseable;

    /** system specific info field */
    GlWindowTech info;

    std::string title;

    double
    /** last time of call to Context::flush() */
        lastFlushTime,
    /** last messured frames per second */
        framesPerSecond;
    /** frames rendered in the current second */
    int framesRendered;

#ifdef MO_ENABLE_FPS_TRACE
    ValueBuffer * fpsTrace;
#endif

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_GLWINDOW_PRIVATE_H

