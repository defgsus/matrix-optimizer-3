/** @file
	x11 system specifics for mag/gl/context.h

	@author def.gsus-
	@version 2012/01/04 moved here
*/

#ifndef MOSRC_GL_X11_GLCONTEXT_X11_H_INCLUDED
#define MOSRC_GL_X11_GLCONTEXT_X11_H_INCLUDED

#ifdef MO_OS_UNIX

#include "include_x11.h"

#undef CursorShape

namespace MO {
namespace GL {

/** structure holding states for the system interaction on X11/GLX */
struct GlContextTech
{
    GlContextTech()
        : display       (nullptr)
        , screen        (0)
        , xwin          (0)
        , glxwin        (0)
        , context       (0)
        , procSwap      (nullptr)
        , procCreateContextAttribs  (nullptr)
    { }

    ::Display *display;
	/** screen handle */
	int screen;
	/** X window handle */
    ::Window xwin;
	/** handle of the GLX extension to a X window */
    ::GLXWindow glxwin;
	/** GLX render context */
    ::GLXContext context;

    ::PFNGLXSWAPINTERVALEXTPROC procSwap;
    ::PFNGLXCREATECONTEXTATTRIBSARBPROC procCreateContextAttribs;
};


} // namespace GL
} // namespace MO

#endif // MO_OS_UNIX

#endif // MOSRC_GL_X11_GLCONTEXT_X11_H_INCLUDED
