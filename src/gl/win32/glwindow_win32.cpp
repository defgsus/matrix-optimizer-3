/** @file
  * GL WINDOW - WIN32 implementation
  *
  * @author def.gsus-
  * @version 2011/12/05 moved to independent file
  * @version 2015/11/15 revised
  *
  */

#ifdef MO_OS_WIN

#ifdef _MSC_VER
// argument conversion, possible loss of data
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)
#endif

#include <cstring>
#include <cstdio>
#include <sstream>
#include <map>
#include <vector>

#include <Windows.h>
#include "gl/window.h"
#include "gl/window_private.h"
#include "tool/mutexlocker.h"
#include "winerror.h"
#include "tool/time.h"
#include "tool/exception.h"
#include "mcw/log.h"

// ---------------------------- callbacks ------------------------------------------

namespace GL {

// forward
int64_t processEvent_(
        Window * win,
        unsigned int uMsg,	// message identifier
        uint64_t wParam,	// first message parameter
        int64_t  lParam 	// second message parameter
        );

#include "wm_codes.inl"

namespace Private {

    /* hwnd_impl_map_ maps between a HWND (os-window handle) and
        the Window class that created the window */

    static std::mutex hwnd_impl_map_mutex_;

    static std::map<const HWND, Window*> hwnd_impl_map_;

    void install_hwnd_(const HWND hwnd, Window* win)
    {
        MutexLocker lock(hwnd_impl_map_mutex_, "HWND:insert-map");
        hwnd_impl_map_[hwnd] = win;
    }

    void uninstall_hwnd_(const HWND hwnd)
    {
        MutexLocker lock(hwnd_impl_map_mutex_, "HWND:release-map");
        hwnd_impl_map_.erase(hwnd);
    }

    Window* get_impl_(const HWND hwnd)
    {
        MutexLocker lock(hwnd_impl_map_mutex_, "HWND:get-map");
        auto i = hwnd_impl_map_.find(hwnd);
        return (i==hwnd_impl_map_.end())? 0 : i->second;
    }

    std::vector<WCHAR> getWChar(const char * t)
    {
        auto s = strlen(t);
        std::vector<WCHAR> v(s+1);
        MultiByteToWideChar(0, 0, t, s, &v[0], s+1);
        return v;
    }

    std::vector<WCHAR> getWChar(const std::string& s)
    {
        std::vector<WCHAR> v(s.size()+1);
        MultiByteToWideChar(0, 0, &s[0], s.size(), &v[0], s.size()+1);
        return v;
    }

    std::string wchar2string(const WCHAR* psz)
    {
        std::string s;
        while (*psz)
        {
            s += *psz;
            ++psz;
        }
        return s;
    }

} // namespace Private


/** window callback function (win32) */
LRESULT CALLBACK WindowProc(
    HWND hwnd,	// handle of window
    UINT uMsg,	// message identifier
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
	//std::cout << "event " << uMsg << " " << wParam << "\n";

	switch (uMsg)
	{
		case WM_CLOSE:
			// TODO
			//DestroyWindow(hwnd);
			return 0;
		break;

        case WM_DESTROY:
			// send a WM_QUIT to the message queue
            PostQuitMessage(0);
            return 0;
		break;

		case WM_SYSCOMMAND:
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				// prevent from happening
				return 0;
			}
		break;
	}

    // default handling
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/** window callback function (win32) */
LRESULT CALLBACK userWindowProc(
    HWND hwnd,	    // handle of window
    UINT uMsg,	    // message identifier
    WPARAM wParam,	// first message parameter
    LPARAM lParam 	// second message parameter
   )
{
    // find the associated WindowImpl
    Window* win = Private::get_impl_(hwnd);

    return (win)
    // dispatch the message to Window class
            ? processEvent_(win, uMsg, wParam, lParam)
    // or to default dispatcher
            : DefWindowProc(hwnd, uMsg, wParam, lParam);
}


/** maps a Win32 keycode to an I::Key::Code */
KeyCode mapWin32KeySym(WPARAM vkey, LPARAM /*keydata*/)
{
	switch (vkey)
	{
        case VK_BACK:       return Key_Backspace;
        case VK_ESCAPE:     return Key_Escape;
        case VK_TAB:        return Key_Tab;
        case VK_RETURN:     return Key_Return;

        case VK_LEFT:       return Key_Left;
        case VK_RIGHT:      return Key_Right;
        case VK_UP:			return Key_Up;
        case VK_DOWN:		return Key_Down;

		case VK_SHIFT:
        case VK_RSHIFT:
        case VK_LSHIFT:		return Key_Shift;
		case VK_MENU:
        case VK_RMENU:
        case VK_LMENU:		return Key_Alt;
		case VK_CONTROL:
        case VK_RCONTROL:
        case VK_LCONTROL:	return Key_Control;

        case VK_PAUSE:		return Key_Pause;
        case VK_CAPITAL:	return Key_CapsLock;
        case VK_NUMLOCK:	return Key_NumLock;
        case VK_SPACE:		return Key_Space;

        case VK_PRIOR:		return Key_PageUp;
        case VK_NEXT:		return Key_PageDown;
        case VK_END:		return Key_End;
        case VK_HOME:		return Key_Home;
        case VK_INSERT:		return Key_Insert;
        case VK_DELETE:		return Key_Delete;

        case VK_MULTIPLY:	return Key_Multiply;
        case VK_DIVIDE:		return Key_Divide;
        case VK_ADD:		return Key_Add;
        case VK_SUBTRACT:	return Key_Subtract;

        case VK_DECIMAL:	return Key_Period;

        case VK_NUMPAD0:	return Key_0;
        case VK_NUMPAD1:	return Key_1;
        case VK_NUMPAD2:	return Key_2;
        case VK_NUMPAD3:	return Key_3;
        case VK_NUMPAD4:	return Key_4;
        case VK_NUMPAD5:	return Key_5;
        case VK_NUMPAD6:	return Key_6;
        case VK_NUMPAD7:	return Key_7;
        case VK_NUMPAD8:	return Key_8;
        case VK_NUMPAD9:	return Key_9;

        case VK_F1:			return Key_F1;
        case VK_F2:			return Key_F2;
        case VK_F3:			return Key_F3;
        case VK_F4:			return Key_F4;
        case VK_F5:			return Key_F5;
        case VK_F6:			return Key_F6;
        case VK_F7:			return Key_F7;
        case VK_F8:			return Key_F8;
        case VK_F9:			return Key_F9;
        case VK_F10:		return Key_F10;
        case VK_F11:		return Key_F11;
        case VK_F12:		return Key_F12;
/*
        case VK_MEDIA_PLAY_PAUSE:	return Key_MediaPlayPause;
        case VK_MEDIA_STOP:			return Key_MediaStop;
        case VK_MEDIA_PREV_TRACK:	return Key_MediaPrevious;
        case VK_MEDIA_NEXT_TRACK:	return Key_MediaNext;
*/
	default:
		// if unknown, return only ascii
        if (vkey>=128) return Key_NoKey;
        if (vkey>='A' && vkey<='Z') vkey += 32;
        return (KeyCode)vkey;
	}
}

MouseKeyCode mapMouseButton(const UINT but)
{
	switch (but)
	{
	    case WM_LBUTTONDOWN:
	    case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK: return MKey_Left; break;

	    case WM_RBUTTONDOWN:
	    case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK: return MKey_Right; break;

	    case WM_MBUTTONDOWN:
	    case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK: return MKey_Middle; break;

        default: return MKey_NoKey; break;
	}
}


/** dispatch events for a win32 window. */
int64_t processEvent_(
        Window * win,
        unsigned int uMsg,      // message identifier
        uint64_t wParam,        // first message parameter
        int64_t lParam          // second message parameter
		)
{
//	std::cout << "dispatch " << get_event_name_(uMsg) << " " << wParam << " " << lParam << "\n";

	bool r;
	switch (uMsg)
	{
		// ------ window specific -------

		case WM_SIZE:
            win->p_w_->onResize();
			return 0;

		case WM_PAINT:
            win->paintEvent();
            //return 0;
			break;

		case WM_CLOSE:
            // ask user event to allow closing
            if ( win->p_w_->isCloseable && win->closeEvent() )
                DestroyWindow(win->p_w_->info.hwnd);
			return 0;

		case WM_DESTROY:
            // notice user
            win->destroyEvent();
			// do the usual stuff
            PostQuitMessage(0);
            win->p_w_->isDestroyed = true;
			return 0;

		// ---- keys -----

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
			// determine ascii or other key
            GetKeyboardState(win->p_w_->info.kbd_state);
            int ret = ToAscii(wParam, (lParam>>16) & 0xff,
                              win->p_w_->info.kbd_state, win->p_w_->info.kbd_chr, 0);
			if (ret == 0)
            {
                //std::cout << "Vkey " << mapWin32KeySym(wParam, lParam) << "\n";
                r = win->keyDownEvent( mapWin32KeySym(wParam, lParam) );
			}
			else
            {	//std::cout << "Akey " << mapWin32KeySym(wParam, lParam)
                // << " " << info.kbd_chr[0] << " " << (info.kbd_chr[0]&0xff) << " " << ret << "\n";
				// do not translate CTRL+...
                if (win->p_w_->info.kbd_state[VK_CONTROL]&128)
                    r = win->keyDownEvent( mapWin32KeySym(wParam, lParam) );
				else
                    r = win->keyDownEvent( /*mapWin32KeySym(wParam, lParam),*/
                                           KeyCode(win->p_w_->info.kbd_chr[0]&0xff) );
			}
            if (r)
                return 0;

            }
        break;

		case WM_SYSKEYUP:
		case WM_KEYUP:

			// determine ascii or other key
            GetKeyboardState(win->p_w_->info.kbd_state);
            if (ToAscii(wParam, (lParam>>16) & 0xff,
                        win->p_w_->info.kbd_state, win->p_w_->info.kbd_chr, 0) == 0)
                r = win->keyUpEvent( mapWin32KeySym(wParam, lParam) );
			else
                r = win->keyUpEvent( /*mapWin32KeySym(wParam, lParam),*/
                                     KeyCode(win->p_w_->info.kbd_chr[0]&0xff) );
            if (r)
                return 0;

        break;

		// -------- focus --------

		case WM_SETFOCUS:
            win->p_w_->isFocus = true;
            win->enterEvent();
			return 0;

		case WM_KILLFOCUS:
            win->p_w_->isFocus =
            win->p_w_->isMouseOver = false;
            win->leaveEvent();
			return 0;

		// ------ mouse --------

		case WM_MOUSELEAVE:
            win->p_w_->isMouseOver = false;
            win->mouseLeaveEvent();
            return 0;

		// --- buttons ---

        //case WM_NCLBUTTONUP:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
        {
            SetCapture(win->p_w_->info.hwnd);
            TrackMouseEvent(&win->p_w_->info.trackmouseevent);
            // translate button
            auto mk = mapMouseButton(uMsg);
            // update window state
            if (!win->p_w_->isMouseOver)
            {
                win->p_w_->isMouseOver = true;
                win->mouseEnterEvent();
            }
            win->p_w_->mouseKeys |= mk;
            // call user event
            r = win->mouseDownEvent( mk );
            if (r)
                return 0;
        }
        break;

		//case WM_NCLBUTTONUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
        {
            ReleaseCapture();
            auto mk = mapMouseButton(uMsg);
            win->p_w_->mouseKeys &= ~mk;
            r = win->mouseUpEvent( mk );
			if (r) return 0;
        }
        break;

		// ---- wheel ----

		case WM_MOUSEWHEEL:
            r = win->mouseWheelEvent( (short)HIWORD(wParam) );
			if (r) return 0;
			break;

		// -- mouse move --

		case WM_MOUSEMOVE:
        {
            if (!win->p_w_->isMouseOver)
            {
                TrackMouseEvent(&win->p_w_->info.trackmouseevent);
                win->p_w_->isMouseOver = true;
                win->mouseEnterEvent();
			}
            int x = GET_X_LPARAM(lParam),
                y = GET_Y_LPARAM(lParam);
            win->p_w_->mouseX = x;
            win->p_w_->mouseY = y;
            r = win->mouseMoveEvent(x, y);
            if (r)
                return 0;
        }
        break;
	}

    return DefWindowProc(win->p_w_->info.hwnd, uMsg, wParam, lParam);
}














void Window::PrivateW::createWindow()
{
    MO_DEBUG2("Creating Window");

	// generate runtime-unique class-name
    // XXX The class name remains 100% unique
    // only until the Window class is destroyed
	std::string class_name;
	{
        std::stringstream s;
        s << "opengl-window-" << std::hex << this << std::dec;
        info.class_name = s.str();
	}

    info.rect.left = curX;
    info.rect.top = curY;
    info.rect.right = curX + curWidth;
    info.rect.bottom = curY + curHeight;

	// window is within this instance
	info.hInstance = info.wc.hInstance
		= GetModuleHandle(NULL);

	info.wc.style =
	// redraw on resize
		CS_HREDRAW | CS_VREDRAW
	// own devicecontex
		| CS_OWNDC
	// receive double clicks
		| CS_DBLCLKS
	// no close icon
//		| CS_NOCLOSE
		;

    auto lclassname = Private::getWChar(info.class_name);

	// no extra bytes on class struct and instance
	info.wc.cbClsExtra = 0;
	info.wc.cbWndExtra = 0;

	info.wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	info.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	info.wc.hbrBackground = 0;

	info.wc.lpszMenuName = 0;
    info.wc.lpszClassName = &lclassname[0];
	// message handler
	info.wc.lpfnWndProc = userWindowProc;

	// mousetrackinginfo
	info.trackmouseevent.cbSize = sizeof(TRACKMOUSEEVENT);
	info.trackmouseevent.dwFlags = TME_LEAVE;
	info.trackmouseevent.dwHoverTime = HOVER_DEFAULT;

	// try to add
	if (!RegisterClass(&info.wc))
        MO_EXCEPTION("Could not register window class");

	// style and exstyle
	info.dwExStyle = 0;//WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	info.dwStyle =
		WS_OVERLAPPEDWINDOW
		// ontop setting
        | (isOnTop * WS_EX_TOPMOST)
		// dont draw on me
		| WS_CLIPSIBLINGS | WS_CLIPCHILDREN
		;

	// increase by frame size
	AdjustWindowRectEx(&info.rect, info.dwStyle, FALSE, info.dwExStyle);
	if (info.rect.left<0)
	{
		info.rect.right -= info.rect.left;
		info.rect.left = 0;
	}
	if (info.rect.top<0)
	{
		info.rect.bottom -= info.rect.top;
		info.rect.top = 0;
	}

	info.hwnd = CreateWindowEx(
		info.dwExStyle,
        &Private::getWChar(info.class_name)[0],
        &Private::getWChar(title)[0],
		info.dwStyle,
		// position
        info.rect.left,
        info.rect.top,
        info.rect.right - info.rect.left,
		info.rect.bottom - info.rect.top,
		// no parent
		HWND_DESKTOP,
		// no menu
        NULL,
		// process instance
		info.hInstance,
		// no child init data
        NULL);

	if (!info.hwnd)
        MO_EXCEPTION("Could not create Window");

    // As suggested by Elvithari
    // http://stackoverflow.com/questions/2656531/tearing-in-opengl
    //SetWindowLong(info.hwnd, GWL_STYLE, 0);

    // install in global list
    Private::install_hwnd_(info.hwnd, pwin);

	// get actual window position and size
	GetWindowRect(info.hwnd, &info.rect);
    curX = oldX = info.rect.left;
    curY = oldY = info.rect.top;
    curWidth = oldWidth = info.rect.right;
    curHeight = oldHeight = info.rect.bottom;

	// keep handle for start mouse tracking
	info.trackmouseevent.hwndTrack = info.hwnd;

    isValid = true;
    isDestroyed = false;

    pwin->show();

    // get style attributes back
    info.dwStyle = GetWindowLong(info.hwnd, GWL_STYLE);

    // get device context
    info.deviceContext = GetDC(info.hwnd);
}





void Window::PrivateW::destroyWindow()
{
    if (!isValid)
        return;

    try
    {
        pwin->destroyEvent();
    }
    catch (...) { }

    isDestroyed = true;
    isValid = false;

	if (info.hwnd)
	try
	{
        DestroyWindow(info.hwnd);
        Private::uninstall_hwnd_(info.hwnd);

        auto lclassname = Private::getWChar(info.class_name);
        UnregisterClass(&lclassname[0], info.hInstance);
	}
	catch (...) { }
}



void Window::PrivateW::onResize()
{
	// get window attributes
	// and copy to local

	GetWindowRect(info.hwnd, &info.rect);
    curX = info.rect.left;
    curY = info.rect.top;

	GetClientRect(info.hwnd, &info.rect);

	// XXX a minimized window will give a 0,0
	// we better avoid sending this value around...
    if (info.rect.right == 0 || info.rect.bottom == 0)
        return;

    curWidth = info.rect.right;
    curHeight = info.rect.bottom;

	// if size changed
    if (oldWidth != curWidth || oldHeight != curHeight)
	// send resize event
        pwin->resizeEvent();
}




void Window::show()
{
    if (!p_w_->isValid)
        return;

    ShowWindow(p_w_->info.hwnd, SW_SHOWNORMAL);
    UpdateWindow(p_w_->info.hwnd);
}

void Window::hide()
{
    if (!p_w_->isValid)
        return;

    ShowWindow(p_w_->info.hwnd, SW_MINIMIZE);
}

void Window::setPosition(int x, int y)
{
    MO_DEBUG2("Window::setPosition(" << x << ", " << y << ")");

    if (!p_w_->isValid)
        return;

    SetWindowPos(p_w_->info.hwnd,
                 HWND_NOTOPMOST,
                 x, y, 0, 0,
                 SWP_ASYNCWINDOWPOS | SWP_NOSIZE);
}

void Window::setSize(int newWidth, int newHeight)
{
    MO_DEBUG2("Window::setSize(" << newWidth << ", " << newHeight << ")");

    if (!p_w_->isValid)
        return;

    SetWindowPos(p_w_->info.hwnd, HWND_NOTOPMOST,
                 0, 0, newWidth, newHeight,
                 SWP_ASYNCWINDOWPOS | SWP_NOMOVE);
}

void Window::setDesktop(unsigned int index)
{
    auto list = getMonitors();
    if (index >= list.size())
    {
        MO_WARN("Desktop index out of range "
                 << index << "/" << list.size());
        return;
    }

    setPosition(list[index].x, list[index].y);
}

void Window::setFullscreen(bool enable)
{
    if (!p_w_->isValid)
        return;

    if (enable)
    {
        MO_DEBUG2("Set window to fullscreen");

        // get window style
        DWORD style = GetWindowLong(p_w_->info.hwnd, GWL_STYLE);
        // store
        p_w_->info.dwStyle = style;

        // clear those attributes
        DWORD clear = WS_CAPTION | WS_BORDER | WS_DLGFRAME
                        | WS_THICKFRAME | WS_SYSMENU | WS_SIZEBOX
                        | WS_HSCROLL | WS_VSCROLL;

        style &= ~clear;
        // apply style
        SetWindowLong(p_w_->info.hwnd, GWL_STYLE, style);

        // get desktop dimensions
        HMONITOR hmon = MonitorFromWindow(
                    p_w_->info.hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(MONITORINFO) };
        if (!GetMonitorInfo(hmon, &mi))
            MO_EXCEPTION("Could not get monitor info");

        // store current dimensions
        RECT rect;
        GetWindowRect(p_w_->info.hwnd, &rect);
        p_w_->oldX = rect.left;
        p_w_->oldY = rect.top;
        p_w_->oldWidth = rect.right - rect.left;
        p_w_->oldHeight = rect.bottom - rect.top;

        // set window to desktop size
        SetWindowPos(p_w_->info.hwnd,
                     HWND_TOPMOST,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_ASYNCWINDOWPOS
                     | SWP_NOZORDER);

        // XXX trying to remove the taskbar here
        // not working when another window is active
        SwitchToThisWindow(p_w_->info.hwnd, TRUE);
        BringWindowToTop(p_w_->info.hwnd);
        SetForegroundWindow(p_w_->info.hwnd);
    }
    else
    {
        MO_DEBUG2("Restore window from fullscreen");
        // return to previous style
        SetWindowLong(p_w_->info.hwnd, GWL_STYLE, p_w_->info.dwStyle);
        // and previous size
        SetWindowPos(p_w_->info.hwnd,
                     HWND_NOTOPMOST,
                     p_w_->oldX,
                     p_w_->oldY,
                     p_w_->oldWidth,
                     p_w_->oldHeight,
                     SWP_ASYNCWINDOWPOS);
    }

    p_w_->isFullscreen = enable;
}



bool Window::update()
{
    if (!p_w_->isValid)
        return false;

	BOOL r;

    while ( (r = PeekMessage(&p_w_->info.msg, p_w_->info.hwnd, 0,0, PM_REMOVE)) != 0 )
	{
	    if (r == -1)
	    {
            MO_ERROR("error on PeekMessage: "
                      << get_event_name_(p_w_->info.msg.message));
	    }
	    else
        {
            // whatever bill does here
            TranslateMessage(&p_w_->info.msg);

            // send further
            DispatchMessage(&p_w_->info.msg);
        }

		// this must stop the callers event-loop
        if (p_w_->isDestroyed)
            return false;
	}

	return true;
}

void Window::swapBuffers()
{
    if (!p_w_->isValid)
        return;

    if (!SwapBuffers(p_w_->info.deviceContext))
        MO_EXCEPTION("Error on SwapBuffers(): '" << getLastWinErrorString() << "'");

    // count fps

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


void Window::setTitle(const char *title)
{
    p_w_->title = title;
    if (!p_w_->isValid)
        return;

    SetWindowText(p_w_->info.hwnd, &Private::getWChar(title)[0]);
}


BOOL CALLBACK monitorInfoProc_(HMONITOR, HDC, LPRECT rect, LPARAM data)
{
    std::vector<MonitorInfo>* infos = (std::vector<MonitorInfo>*)(data);

    MonitorInfo info;
    info.x = rect->left;
    info.y = rect->top;
    info.width = rect->right - rect->left;
    info.height = rect->bottom - rect->top;

    infos->push_back(info);

    return TRUE;
};

std::vector<MonitorInfo> Window::getMonitors()
{
    std::vector<MonitorInfo> infos;

#if 0
    // number of monitors
    int count = getSystemMetrics(SM_CMONITORS);

    for (int i=0; i<count; ++i)
    {
        MonitorInfo info;
        info.x =
        infos.push_back(info);
    }

#else
    EnumDisplayMonitors(NULL, NULL, monitorInfoProc_, (LPARAM)&infos);

    // set index and get each display name
    int k = 0;
    for (MonitorInfo & m : infos)
    {
        m.index = k++;

        RECT r;
        r.left = m.x;
        r.right = m.x + m.width;
        r.top = m.y;
        r.bottom = m.y + m.height;
        auto hmon = MonitorFromRect(&r, MONITOR_DEFAULTTONULL);
        if (hmon == NULL)
            continue;

        MONITORINFOEX mi;
        mi.cbSize = sizeof(MONITORINFOEX);
        if (!GetMonitorInfo(hmon, &mi))
            continue;

        m.name = Private::wchar2string( mi.szDevice );
    }
#endif

    return infos;
}


} // namespace GL

#endif // MO_OS_WIN
