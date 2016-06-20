/** @file
  * <h2>GLUSE WINDOW</h2>
  *
  * @author def.gsus-
  * @version 2011/06/20 v0.1
  * @version 2011/06/22 v0.11 started x11 part
  * @version 2015/11/10 refactured class and revised events
  *
  * @code
  * // usage:
  * Window win(640,480);
  * while (win->update()) ...;
  * @endcode
  *
  */
#ifndef MOSRC_GL_GLWINDOW_H_INCLUDED
#define MOSRC_GL_GLWINDOW_H_INCLUDED

#include <cinttypes>
#include <string>
#include <vector>

namespace MO {
namespace GL {

/** <b>enumeration of standard ascii keycodes + common extended keycodes </b>

    <p>everything up to Key_Delete is mapped into ascii space,
    characters (like Key_A) are mapped to lower-case ascii characters.</p>

    <p>extended keycodes are arbitrarily ordered.</p>
 */
enum KeyCode
{
    Key_NoKey,

    Key_Backspace = 8,
    Key_Tab,
    Key_Enter,
    Key_VTab,
    Key_FF, 			// form feed
    Key_Return,			// carriage return

    Key_Escape = 27,

    Key_Space = 32,
    Key_Excl, 			// !
    Key_DoubleQuote, 	// "
    Key_Number,			// #
    Key_Dollar,			// $
    Key_Percent,		// %
    Key_Amp,			// &
    Key_Quote,			// '
    Key_ParOpen,		// (
    Key_ParClose,		// )
    Key_Multiply,		// *
    Key_Add,			// +
    Key_Comma,			// ,
    Key_Subtract,		// -
    Key_Period,			// .
    Key_Divide,			// /

    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,

    Key_Colon,			// :
    Key_Semicolon,		// ;
    Key_LessThan,		// <
    Key_Equal,			// =
    Key_GreaterThan,	// >
    Key_Question,		// ?
    Key_At,				// @

    Key_BracketOpen = 	'[',
    Key_Backslash,		/* \ */
    Key_BracketClose,	// ]
    Key_Caret,			// ^
    Key_Underscore,		// _
    Key_Accent,			// `

    // map to lower case
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,

    Key_BraceOpen,		// {
    Key_Bar,			// |
    Key_BraceClose,		// }
    Key_Tilde,			// ~

    Key_Delete = 127,	// last ascii character

    Key_Up,
    Key_Down,
    Key_Left,
    Key_Right,

    Key_PageUp,
    Key_PageDown,
    Key_Home,
    Key_End,
    Key_Insert,

    Key_Control,
    Key_Alt,
    Key_Shift,

    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,

    Key_CapsLock,
    Key_NumLock,

    Key_Pause,
    Key_Super,
    Key_Menu,
    Key_Meta,

    // media play-bar
    Key_MediaPlayPause,
    Key_MediaStop,
    Key_MediaPrevious,
    Key_MediaNext,

    Key_MAX
};

enum MouseKeyCode
{
    MKey_NoKey,

    MKey_Left       = 1,
    MKey_Right      = 2,
    MKey_Middle     = 4
};

/** Or-combination of MouseKeyCode */
typedef int MouseKeyCodes;


// forward
struct GlWindowTech;
struct MonitorInfo;

/** A class completely maintaining a system window and it's events.
    <p>This is independent of Qt for more efficient low-level handling.</p>
    <p>The window is created with proper opengl capability.</p>
 */
class GlWindow
{
public: // ------------- ctor --------------------

	/** constructor that creates a window with certain size */
    GlWindow(int width = 320, int height = 320)
        : GlWindow(width, height, 0, 0)
    { }

    /** constructor that creates a window with certain size and position */
    GlWindow(int width, int height, int offx, int offy);

	/** default destructor */
    virtual ~GlWindow();

    // ----------- static interface --------------

    //static std::vector<MonitorInfo> getMonitors();

    // -------------- getter ---------------------

    /** Returns true when the window is created */
    bool isOk() const;

    /** Returns true when the mouse is currently inside the client area */
    bool isMouseOver() const;

    /** Returns true when this window has input/keyboard focus */
    bool isFocus() const;

    /** Returns true when window is in fullscreen mode. */
    bool isFullscreen() const;

    /** Can the window be closed by user? */
    bool isCloseable() const;

    /** Returns the current width */
    int width() const;
    /** Return the current height */
    int height() const;

    /** Returns the current title of the window */
    const std::string& title() const;

    /** Returns a struct with system specific information.
        WindowTech is defined in gl/win32/window.h or gl/x11/window.h */
    const GlWindowTech& info() const;

    /** Returns current mouse x in pixels */
    int mouseX() const;
    /** Returns current mouse y in pixels */
    int mouseY() const;
    /** Returns or combination of currently pressed mouse keys */
    MouseKeyCodes mouseKeys() const;

    // ------------ setter ---------------------

    /** Tries to set position of window */
    void setPosition(int x, int y);

    /** Tries to set new size of window */
    void setSize(int newWidth, int newHeight);

    /** Set or unset fullscreen mode. */
    void setFullscreen(bool enable);

    /** Moves the window to the given desktop */
    void setDesktop(unsigned int index);

    /** If valid then shows window, else ignored */
    void show();

    /** Hides/minimizes the window */
    void hide();

    /** Sets the window title bar text */
    void setTitle(const char *title);

    /** Allow user to close window */
    void setCloseable(bool enable);

    /** Returns a writeable struct with system specific information.
        WindowTech is defined in gl/win32/window.h or gl/x11/window.h */
    GlWindowTech& info();

    // ------------- events -----------------

    /** Polls messages from the que.
        returns false when window got closed! */
	bool update();

    // ------------- rendering --------------

    /** Swaps front and back buffer and messures fps */
    void swapBuffers();

    /** Returns the aquired frames per second.
        This value is updated by the swapBuffers() method */
    double fps() const;

#ifdef MO_ENABLE_FPS_TRACE
    /** Returns a trace of each previous frame's fps */
    const ValueBuffer* fpsTrace() const;
    /** Returns write access to the fps tracer */
    ValueBuffer* fpsTrace();
#endif

protected:

    virtual void resizeEvent() { }
    virtual bool closeEvent() { return true; }
    virtual void destroyEvent() { }

    virtual void paintEvent() { }

    virtual void enterEvent() { }
    virtual void leaveEvent() { }

    virtual bool keyDownEvent(KeyCode) { return false; }
    virtual bool keyUpEvent(KeyCode) { return false; }

    virtual void mouseEnterEvent() { }
    virtual void mouseLeaveEvent() { }
    virtual bool mouseDownEvent(MouseKeyCode) { return false; }
    virtual bool mouseUpEvent(MouseKeyCode) { return false; }
    virtual bool mouseWheelEvent(short delta) { (void)delta; return false; }
    virtual bool mouseMoveEvent(int x, int y) { (void)x; (void)y; return false; }

private:

    /** disable copy */	GlWindow(GlWindow&) = delete;
    /** disable copy */	void operator=(GlWindow&) = delete;

#ifdef MO_OS_WIN
    friend int64_t processEvent_(
            GlWindow * win, unsigned int uMsg, uint64_t wParam, int64_t lParam);
#elif defined(MO_OS_UNIX)
    friend void XWindowEventDispatcher(void* xevent, GlWindow* win);
#endif

    struct PrivateW;
    PrivateW * p_w_;
};



} // namespace GL
} // namespace MO


#endif // MOSRC_GL_GLWINDOW_H_INCLUDED







