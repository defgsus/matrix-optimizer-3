/** @file
  * GL WINDOW - WIN32 header
  *
  * @author def.gsus-
  * @version 2011/12/05 moved to independent file
  *
  */
#ifndef MAG_GL_WIN32_WINDOW_H_INCLUDED
#define MAG_GL_WIN32_WINDOW_H_INCLUDED

#ifdef MO_OS_WIN

#include <string>

#include <windows.h>
#include <winuser.h>
#include <windowsx.h>

namespace GL {

/** Structure holding states for the system interaction on win32 */
struct WindowTech
{
	/** window handle */
	HWND		hwnd;
	/** application instance */
	HINSTANCE	hInstance;
	/** message temp */
    MSG             msg;

	/** window class structure */
    WNDCLASS	wc;
	/** window style */
	DWORD		dwStyle;
	/** extended style */
	DWORD		dwExStyle;
	/** stupid window rect struct */
	RECT		rect;
    /** Device context from GetDC() */
    HDC         deviceContext;

	/** mouse event tracking defines */
	TRACKMOUSEEVENT trackmouseevent;

	/** unique class-name for every window */
	std::string class_name;

	/** the keyboard state */
	BYTE kbd_state[256];

	/** keyboard input buffer */
	WORD kbd_chr[2];
};


} // namespace GL

#endif // MO_OS_WIN

#endif // MAG_GL_WIN32_WINDOW_H_INCLUDED
