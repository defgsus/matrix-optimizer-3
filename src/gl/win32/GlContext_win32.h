/** @file
	win32 system specifics for mag/gl/context.h

	@author def.gsus-
	@version 2012/01/04 moved here
*/
#ifndef MAG_GL_WIN32_CONTEXT_H_INCLUDED
#define MAG_GL_WIN32_CONTEXT_H_INCLUDED

#include <set>
#include <Windows.h>

namespace MO {
namespace GL {

struct WglExt;

/** structure holding states for the system interaction on windows */
struct GlContextTech
{
    GlContextTech()
        : hrc       (0)
        , hdc       (0)
        , hwnd      (0)
        , pfdnr     (0)
        , wglExt    (nullptr)
    { }

	/** rendering context */
	HGLRC		hrc;
	/** device context */
    HDC 		hdc;
	/** window handle */
	HWND		hwnd;
    /** as typename suggests */
	PIXELFORMATDESCRIPTOR pfd;
    /** The index of the pixel format */
    int pfdnr;
    /** Set of HDCs that have been assigned to this context,
        e.g. via Context::makeCurrent(Window*) */
    std::set<HDC> assignedHdc;

    /** wgl extension functions */
    WglExt * wglExt;
};

} // namespace GL
} // namespace MO

#endif // MAG_GL_WIN32_CONTEXT_H_INCLUDED
