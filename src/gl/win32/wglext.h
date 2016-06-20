#ifndef MAG_GL_WIN32_WGLEXT_H
#define MAG_GL_WIN32_WGLEXT_H

#include <Windows.h>

#include "gl/opengl.h"

namespace GL {

// --- wgl extensions ---
#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092

struct WglExt
{
    typedef HGLRC (*PFNwglCreateContextAttribsARB)(HDC, HGLRC, const int*);
    typedef BOOL (*PFNwglSwapIntervalEXT)(int);

    // swapgroups extension
    // https://www.opengl.org/registry/specs/NV/wgl_swap_group.txt
    typedef BOOL (*PFNwglJoinSwapGroupNV)(HDC, GLuint);
    typedef BOOL (*PFNwglBindSwapBarrierNV)(GLuint, GLuint);
    typedef BOOL (*PFNwglQuerySwapGroupNV)(HDC, GLuint*, GLuint*);
    typedef BOOL (*PFNwglQueryMaxSwapGroupsNV)(HDC, GLuint*, GLuint*);
    typedef BOOL (*PFNwglQueryFrameCountNV)(HDC, GLuint*);
    typedef BOOL (*PFNwglResetFrameCountNV)(HDC);

    PFNwglCreateContextAttribsARB wglCreateContextAttribsARB;
    PFNwglSwapIntervalEXT wglSwapIntervalEXT;

    PFNwglJoinSwapGroupNV wglJoinSwapGroupNV;
    PFNwglBindSwapBarrierNV wglBindSwapBarrierNV;
    PFNwglQuerySwapGroupNV wglQuerySwapGroupNV;
    PFNwglQueryMaxSwapGroupsNV wglQueryMaxSwapGroupsNV;
    PFNwglQueryFrameCountNV wglQueryFrameCountNV;
    PFNwglResetFrameCountNV wglResetFrameCountNV;
};

} // namespace GL


#endif // MAG_GL_WIN32_WGLEXT_H

