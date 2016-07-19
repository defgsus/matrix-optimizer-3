/** @file
  * GL WINDOW - X11 header
  *
  * @author def.gsus-
  * @version 2011/12/05 moved to independent file
  *
  */
#ifndef MOSRC_GL_X11_GLWINDOW_H_INCLUDED
#define MOSRC_GL_X11_GLWINDOW_H_INCLUDED

#ifdef MO_OS_UNIX

#include "gl/opengl.h"
#include "include_x11.h"

namespace MO {
namespace GL {


/** structure holding states for the system interaction on Linux/X11/GLX */
struct GlWindowTech
{
	/** X display handle */
    ::Display *display;
	/** screen handle */
	int screen;
	/** root window handle */
    ::Window rootwin;
	/** window handle */
    ::Window win;
    /** Context wrapper for this window */
    ::GLXWindow glxwin;

	/** visual info of window */
    ::XVisualInfo *vinfo;
	/** window attributes */
    ::XSetWindowAttributes wina;

	/** frame buffer configurations */
    ::GLXFBConfig	*fbconfig;

	/** buffer */
    ::XEvent event;

	/** smallest keyboard symbol. @see WindowTechX::keymap */
	int min_keysym,
	/** largest keyboard symbol. @see WindowTechX::keymap */
		max_keysym,
	/** nr of keysymbols per keycode */
		keysym_per_keycode;
	/** key symbols as returned by XGetKeyboardMapping() */
    ::KeySym *keymap;

	/** an delete window atom */
    ::Atom wm_delete_window;

	/** hints for window manager */
    ::XWMHints *wmhints;
};


} // namespace GL
} // namespace MO


#endif // MO_OS_UNIX

#endif // MOSRC_GL_X11_GLWINDOW_H_INCLUDED
