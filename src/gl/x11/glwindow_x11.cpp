/** @file
  * GL WINDOW - X11 implementation
  *
  * @author def.gsus-
  * @version 2011/12/05 moved to independent file
  * @version 2015/11/24 revised
  *
  */

#ifdef MO_OS_UNIX

#include "gl/glwindow_private.h"
#include "io/time.h"
#include "io/error.h"
#include "io/log_gl.h"

namespace MO {
namespace GL {

namespace
{
    /** predefined attribute for double buffered frame buffer */
    int fb_attrib[] = {
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER,  True,
        // max bits
        GLX_RED_SIZE,      1,
        GLX_GREEN_SIZE,    1,
        GLX_BLUE_SIZE,     1,
        XX::X_None
    };
}

// ---------------------------- callbacks ------------------------------------------


/** window callback for a (GL)X window */
static int WindowProc( Display* /*dpy*/, XEvent *event, XPointer arg )
{
    //MO_PRINT( "etype " << event->type );

	// acknowledge mapping event
    return ( (event->type == XX::X_MapNotify)
             && (event->xmap.window == (::Window) arg) );
}

/** maps an XKeySym to an I::Key::Code */
KeyCode mapXKeySym(unsigned int xkeysym)
{
	switch (xkeysym)
	{
        case XX::X_NoSymbol: return Key_NoKey;

		/* although some of the following live in ascii space
			they get remapped here to be sure */
        case XK_Escape: return Key_Escape;
        case XK_Return: return Key_Enter;
        case XK_KP_Space: return Key_Space;
        case XK_Tab: return Key_Tab;
        case XK_Home: return Key_Home;
        case XK_End: return Key_End;
        case XK_Page_Up: return Key_PageUp;
        case XK_Page_Down: return Key_PageDown;

        case XK_Insert: return Key_Insert;
        case XK_Delete: return Key_Delete;
        case XK_BackSpace: return Key_Backspace;

		case XK_Control_L:
        case XK_Control_R: return Key_Control;
		case XK_Alt_L:
        case XK_Alt_R: return Key_Alt;
		case XK_Shift_L:
        case XK_Shift_R: return Key_Shift;

        case XK_KP_Equal: return Key_Equal;
        case XK_KP_Add: return Key_Add;
        case XK_KP_Multiply: return Key_Multiply;
        case XK_KP_Subtract: return Key_Subtract;
        case XK_KP_Divide: return Key_Divide;

        case XK_KP_Decimal: return Key_Period;
        case XK_KP_Separator: return Key_Comma;

        case XK_Up: return Key_Up;
        case XK_Down: return Key_Down;
        case XK_Left: return Key_Left;
        case XK_Right: return Key_Right;

        case XK_F1: return Key_F1;
        case XK_F2: return Key_F2;
        case XK_F3: return Key_F3;
        case XK_F4: return Key_F4;
        case XK_F5: return Key_F5;
        case XK_F6: return Key_F6;
        case XK_F7: return Key_F7;
        case XK_F8: return Key_F8;
        case XK_F9: return Key_F9;
        case XK_F10: return Key_F10;
        case XK_F11: return Key_F11;
        case XK_F12: return Key_F12;

        case XK_Mode_switch: return Key_CapsLock;
        case XK_Num_Lock: return Key_NumLock;
        //case XK_: return Key_; break;
	default:
		// return only ascii
		//if (xkeysym>127) std::cout << "xkeysym exceeded " << xkeysym << "\n";
        //return (Key_Code)XKeysymToKeycode(win->info.display, xkeysym);
        //return (Key_Code)XLookupKeysym(win->info.display, xkeysym);
        return (xkeysym < 128) ? (KeyCode)xkeysym : Key_NoKey;
	}
}

MouseKeyCode mapXMouseButton(unsigned int key)
{
    switch (key)
    {
        case XX::X_Button1: return MKey_Left;
        case XX::X_Button2: return MKey_Middle;
        case XX::X_Button3: return MKey_Right;
        default: return MKey_NoKey;
    }
}


/** dispatch events for a X11 window */
void XWindowEventDispatcher(void* xevent, GlWindow* win)
{
    if (!win)
        return;

    auto event = static_cast<XEvent*>(xevent);

    //MO_PRINT("event " << event->type);

    // remap events
    switch (event->type)
    {
		// notify on mapping
        case XX::X_MapNotify:
			//std::cout << "map\n";
		break;

        case XX::X_UnmapNotify:
			//std::cout << "unmap\n";
		break;

		// visibility
        case XX::X_VisibilityNotify:
		break;

        case XX::X_DestroyNotify:
            MO_DEBUG_GL("DestroyNotify");
            win->destroyEvent();
            win->p_w_->isDestroyed = true;
		break;

		// window enter / exit

        case XX::X_EnterNotify:
            win->p_w_->isFocus = true;
            win->enterEvent();
		break;
        case XX::X_LeaveNotify:
            win->p_w_->isFocus = false;
            win->leaveEvent();
		break;

		// USER INPUT

        case XX::X_KeyPress:
    	{
			// check keycode range (should be ok though)
            if (event->xkey.keycode < (unsigned int)win->p_w_->info.min_keysym ||
                event->xkey.keycode > (unsigned int)win->p_w_->info.max_keysym)
                break;

			// call user interface with corrected keycode
            int shif = ((event->xkey.state & XX::X_ShiftMask)
                        || (event->xkey.state & XX::X_LockMask));
			int k = XLookupKeysym(&event->xkey, shif);
            win->keyDownEvent( mapXKeySym(k) );
					/*win->info.keymap[ (event->xkey.keycode - win->info.min_keysym)
										* win->info.keysym_per_keycode + shif ] ) );*/
    	}
		break;

        case XX::X_KeyRelease:
    	{
			// check keycode range (should be ok though)
            if (event->xkey.keycode < (unsigned int)win->p_w_->info.min_keysym ||
                event->xkey.keycode > (unsigned int)win->p_w_->info.max_keysym)
                break;

			// call user interface with corrected keycode
            int shif = ((event->xkey.state & XX::X_ShiftMask)
                        || (event->xkey.state & XX::X_LockMask));
			int k = XLookupKeysym(&event->xkey, shif);
            win->keyUpEvent( mapXKeySym(k) );
					/*win->info.keymap[ (event->xkey.keycode - win->info.min_keysym)
									 * win->info.keysym_per_keycode ]));*/
    	}
		break;

        case XX::X_ButtonPress:
        {
            // update mouse-over state
            if (!win->p_w_->isMouseOver)
            {
                win->p_w_->isMouseOver = true;
                win->mouseEnterEvent();
            }
            if (event->xbutton.button == XX::X_Button4)
                win->mouseWheelEvent(1);
            else if (event->xbutton.button == XX::X_Button5)
                win->mouseWheelEvent(-1);
            else
            {
                // translate button
                auto mk = mapXMouseButton(event->xbutton.button);

                win->p_w_->setMouseKey(mk, true);
                // call user event
                win->mouseDownEvent( mk );
            }
        }
		break;

        case XX::X_ButtonRelease:
        {
            // translate button
            auto mk = mapXMouseButton(event->xbutton.button);
            // update window state
            win->p_w_->setMouseKey(mk, false);
            // call user event
            win->mouseUpEvent( mk );
        }
		break;

        case XX::X_MotionNotify:
            if (!win->p_w_->isMouseOver)
            {
                win->p_w_->isMouseOver = true;
                win->mouseEnterEvent();
            }
            win->p_w_->mouseXDelta = win->p_w_->mouseX - event->xmotion.x;
            win->p_w_->mouseYDelta = win->p_w_->mouseY - event->xmotion.y;
            win->p_w_->mouseX = event->xmotion.x;
            win->p_w_->mouseY = event->xmotion.y;
            win->mouseMoveEvent(event->xmotion.x, event->xmotion.y);
		break;

        case XX::X_KeymapNotify:
		break;

        case XX::X_ResizeRequest:
            //win->p_w_->onResize();
		break;

        case XX::X_ConfigureNotify:
            win->p_w_->onResize();
		break;

	}

}

/** Wraps the call to XWindowEventDispatcher */
void XWindowEventDispatcherWrapper(XEvent* event, GlWindow* win)
{
    XWindowEventDispatcher(static_cast<void*>(event), win);
}







void GlWindow::PrivateW::createWindow()
{
    MO_DEBUG_GL("Creating Window");

    // get display device
	if(!(info.display = XOpenDisplay(0)))
        MO_GL_ERROR("GlWindow:: failed on XOpenDisplay(...)");

	// grab some handles
	info.screen = DefaultScreen(info.display);
	info.rootwin = RootWindow(info.display, info.screen);

	// query GLX

	int error, event, num;
	if (!glXQueryExtension(info.display, &error, &event))
        MO_GL_ERROR("GlWindow:: failed on glXQueryExtension(...)");

    MO_DEBUG_GL("GlWindow:: using glx server: "
		<< glXQueryServerString(info.display, info.screen, GLX_VENDOR)
        << " v" << glXQueryServerString(info.display, info.screen, GLX_VERSION)
               );

	// find matching framebuffer config

	info.fbconfig = glXChooseFBConfig(
        info.display, info.screen, fb_attrib, &num
		);

	if (!info.fbconfig)
        MO_GL_ERROR("GlWindow::failed on glXChooseFBConfig(...)");

	// create X visual info from first returned fbconfig

	info.vinfo = glXGetVisualFromFBConfig( info.display, info.fbconfig[0] );


	// X window attributes

	memset(&info.wina, 0, sizeof(XSetWindowAttributes));
	info.wina.border_pixel = 0;

	// define wanted events
	info.wina.event_mask =
          StructureNotifyMask
		| KeyPressMask
		| KeyReleaseMask
		| ButtonPressMask
		| ButtonReleaseMask
		| EnterWindowMask
		| LeaveWindowMask
		| PointerMotionMask
		| Button1MotionMask
		| Button2MotionMask
		| Button3MotionMask
		| Button4MotionMask
		| Button5MotionMask
		| ButtonMotionMask
		| KeymapStateMask
		| ExposureMask
		| VisibilityChangeMask
		| FocusChangeMask
		;

	info.wina.do_not_propagate_mask = NoEventMask;

	// match colormap
	info.wina.colormap =
		XCreateColormap( info.display, info.rootwin, info.vinfo->visual, AllocNone );


	// create the X window
	info.win = XCreateWindow(
			info.display, info.rootwin,
            curX, curY, curWidth, curHeight,
			// boarder width
			0,
			info.vinfo->depth,
			InputOutput,
			info.vinfo->visual,
			CWBorderPixel | CWColormap | CWEventMask | CWDontPropagate,
			&info.wina
			);

	if (!info.win)
        MO_GL_ERROR("GlWindow:: failed on XCreateWindow(...)");

	// set title
    XStoreName(info.display, info.win, title.c_str());

	// state which window events we want to receive
	//XSelectInput(info.display, info.win, info.wina.event_mask);

	// -- set wm hints --

	info.wmhints = XAllocWMHints();
	info.wmhints->initial_state = NormalState;
    info.wmhints->input = True;
    info.wmhints->flags = StateHint | InputHint;
    XSetWMHints(info.display, info.win, info.wmhints);



	// get keysymbol map
	XDisplayKeycodes(info.display, &info.min_keysym, &info.max_keysym);
	info.keymap = XGetKeyboardMapping(
		info.display, info.min_keysym, info.max_keysym - info.min_keysym + 1,
		&info.keysym_per_keycode);


	// create and register delete window atom
	info.wm_delete_window =
        XInternAtom (info.display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols (info.display, info.win, &info.wm_delete_window, 1);


	// enable the window...
	XMapWindow(info.display, info.win);
	// ...and wait to appear
	XIfEvent( info.display, &info.event, WindowProc, (XPointer) info.win );

    isValid = true;
    pwin->show();
}


void GlWindow::PrivateW::destroyWindow()
{
    if (!isValid)
        return;

	if (info.win)
	{
        MO_DEBUG_GL("XDestroyWindow()");
        //XUnmapWindow(info.display, info.win);
		XDestroyWindow(info.display, info.win);

        info.win = 0;
	}
    isValid = false;
    XFlush(info.display);

    /** @todo XCloseDisplay gives segfault */
	//XCloseDisplay(info.display);

	// free keysymbols
    if (info.keymap)
        XFree(info.keymap);
    info.keymap = 0;

    //if (info.wmhints)
    //    XFreeWMHints(info.wmhints);

}






void GlWindow::PrivateW::onResize()
{
	// get attributes
	XWindowAttributes wa;
	XGetWindowAttributes(info.display, info.win, &wa);

	// copy them to local
    curX = wa.x;
    curY = wa.y;
    curWidth = wa.width;
    curHeight = wa.height;

    pwin->resizeEvent();
}













/** @todo x11 GlWindow::show() */
void GlWindow::show()
{

}


/** @todo x11 Window setPosition() */
void GlWindow::setPosition(int x, int y)
{
    XMoveWindow(p_w_->info.display, p_w_->info.win, x, y);

}

/** @todo x11 GlWindow::size() */
void GlWindow::setSize(int newWidth, int newHeight)
{
    XResizeWindow(p_w_->info.display, p_w_->info.win, newWidth, newHeight);
}


/** @todo x11 GlWindow::fullscreen() */
void GlWindow::setFullscreen(bool enable)
{
    if (!isOk() || enable == p_w_->isFullscreen)
        return;

	/* partly from http://tonyobryan.com/index.php?article=9 */

	struct Hints
	{
		unsigned long
			flags,
			functions,
			decorations;
		long
			inputMode;
		unsigned long
			status;
	};

	Hints hints;
    Atom property;

    memset(&hints, 0, sizeof(Hints));

    auto & info = p_w_->info;

	if (enable)
	{
        // store current size/pos
        XWindowAttributes wa;
        XGetWindowAttributes(info.display, info.win, &wa);
        p_w_->oldX = wa.x; // XXX These are wrong
        p_w_->oldY = wa.y;
        p_w_->oldWidth = wa.width;
        p_w_->oldHeight = wa.height;
        //MO_INFO(p_w_->oldX << " " << p_w_->oldY
        //         << " " << p_w_->oldWidth << " " << p_w_->oldHeight);

        // ---- remove boarders and decoration ----

        hints.flags = 2; // set decoration
        hints.decorations = 0; // switch off

        property = XInternAtom(info.display, "_MOTIF_WM_HINTS", True);
		if (property == 0)
            MO_GL_ERROR("ERROR: GlWindow::fullscreen() / XInternAtom()");

		XChangeProperty(info.display, info.win, property, property, 32,
						PropModeReplace, (unsigned char *)&hints, 5);

		// --- resize and ontopify ----

        // get desktop size
        const int screenNum = info.vinfo->screen;
        const ::Screen* scr = ScreenOfDisplay(p_w_->info.display, screenNum);
        p_w_->curWidth = WidthOfScreen(scr);
        p_w_->curHeight = HeightOfScreen(scr);

        XMoveResizeWindow(info.display, info.win, 0,0, p_w_->curWidth, p_w_->curHeight);
		XMapRaised(info.display, info.win);
        //XGrabPointer(info.display, info.win, True, 0, GrabModeAsync, GrabModeAsync, info.win, 0L, CurrentTime);
        //XGrabKeyboard(info.display, info.win, False, GrabModeAsync, GrabModeAsync, CurrentTime);

        p_w_->isFullscreen = true;
	}
    else
    {
        // ---- restore boarders and decoration ----

        hints.flags = 2; // set decoration
        hints.decorations = 0xffffffff; // switch all on (just guessing here)

        property = XInternAtom(info.display, "_MOTIF_WM_HINTS", True);
        if (property == 0)
            MO_GL_ERROR("ERROR: GlWindow::fullscreen() / XInternAtom()");

        XChangeProperty(info.display, info.win, property, property, 32,
                        PropModeReplace, (unsigned char *)&hints, 5);

        // change back to previous pos/size

        XMoveResizeWindow(info.display, info.win,
                          p_w_->oldX, p_w_->oldY,
                          p_w_->oldWidth, p_w_->oldHeight);
        p_w_->isFullscreen = false;
    }
}



bool GlWindow::update()
{
    if (!isOk())
        return false;

	// check window close

    if ( (XCheckTypedEvent(p_w_->info.display, XX::X_ClientMessage, &p_w_->info.event))
        && (p_w_->info.event.xclient.window == p_w_->info.win)
        && (p_w_->info.event.xclient.message_type = p_w_->info.wm_delete_window)
        // see if client agrees
        && (closeEvent()) )
	{
		// method takes care of notifing clients
        p_w_->destroyWindow();
	}


	// flush event cue
    XFlush(p_w_->info.display);

	// loop through the event que / poll mode
    while (XCheckMaskEvent(p_w_->info.display,
                           p_w_->info.wina.event_mask, &p_w_->info.event))
	{
		//std::cout << info.event.type << "\n";

		// send event further for dispatching to other clients
        XWindowEventDispatcherWrapper(&p_w_->info.event, this);
	}

	// if we got here, events are processed and window is still active
	return true;
}

void GlWindow::swapBuffers()
{
    if (!p_w_->isValid)
        return;

    glXSwapBuffers( p_w_->info.display, p_w_->info.glxwin );

    // count fps

    ++p_w_->framesRendered;

    double ti = systemTime();
    if (p_w_->lastFlushTime > 0. && ti > p_w_->lastFlushTime)
    {
        p_w_->framesPerSecond = 1. / (ti - p_w_->lastFlushTime);
#ifdef MO_ENABLE_FPS_TRACE
        p_w_->fpsTrace->addValue(p_w_->framesPerSecond);
#endif
    }
    p_w_->lastFlushTime = ti;
}


void GlWindow::setTitle(const char *title)
{
    if (!isOk())
        return;

    XStoreName(p_w_->info.display, p_w_->info.win, title);
}

/** @todo x11 GlWindow::setDesktop */
void GlWindow::setDesktop(unsigned int)
{

}

/** @todo x11 GlWindow::getMonitors() order/alignment */
#if 0
std::vector<MonitorInfo> GlWindow::getMonitors()
{
    std::vector<MonitorInfo> list;

    ::Display* display;
    if(!(display = XOpenDisplay(0)))
        MO_GL_ERROR("GlWindow:: failed on XOpenDisplay(...) for getMonitors");

    int num = ScreenCount(display);
    for (int i=0; i<num; ++i)
    {
        ::Screen* scr = ScreenOfDisplay(display, i);
        MonitorInfo inf;
        inf.clear();
        inf.width = WidthOfScreen(scr);
        inf.height = HeightOfScreen(scr);
        inf.index = i;
        list.push_back(inf);
    }

    return list;
}
#endif


} // namespace GL
} // namespace MO

#endif // MO_OS_UNIX
