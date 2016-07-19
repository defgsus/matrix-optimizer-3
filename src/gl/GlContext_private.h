#ifndef MOSRC_GL_GLCONTEXT_PRIVATE_H
#define MOSRC_GL_GLCONTEXT_PRIVATE_H

#include "gl/GlContext.h"

#ifdef MO_OS_WIN
#   include "gl/win32/glcontext_win32.h"
#elif defined(MO_OS_UNIX)
#   include "gl/x11/GlContext_x11.h"
#else
#   error undefined OS
#endif

namespace MO {
namespace GL {

struct GlContext::Private
{
    Private(GlContext * p)
        : pcontext          (p)
        , isValid           (false)
        , window            (nullptr)
        , syncMode          (SYNC_FIXED)
        , vMajor            (1)
        , vMinor            (1)
    {

    }

    ~Private()
    {
    }
#ifdef MO_OS_WIN
    void swapBuffers(HDC hdc);
#endif
    /** Link to class */
    GlContext * pcontext;

    bool
        /** anything set up? */
        isValid;

    /** system specific tech speak */
    GlContextTech info;

    /** nonzero when a window is assigned */
    GlWindow *window;

    Sync syncMode;

    int vMajor, vMinor;

};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_GLCONTEXT_PRIVATE_H

