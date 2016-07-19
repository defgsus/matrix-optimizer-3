#include <iostream>
#include <cstring>

#include <Qt> // for keycodes

#include "GlWindow.h"
#include "GlWindow_private.h"
#include "io/error.h"
#include "io/log_gl.h"


// ------------------ system independend implementation -------------

namespace MO {
namespace GL {

GlWindow::GlWindow(int width, int height, int offx, int offy)
    : p_w_      (new PrivateW(this))
{
    p_w_->curWidth = width;
    p_w_->curHeight = height;
    p_w_->curX = offx;
    p_w_->curY = offy;
    //p_w_->parent = parent;
    p_w_->createWindow();
}

GlWindow::~GlWindow()
{
    p_w_->destroyWindow();
    delete p_w_;
}


bool GlWindow::isOk() const { return p_w_->isValid && !p_w_->isDestroyed; }
bool GlWindow::isFocus() const { return p_w_->isFocus; }
bool GlWindow::isFullscreen() const { return p_w_->isFullscreen; }
bool GlWindow::isMouseOver() const { return p_w_->isMouseOver; }
bool GlWindow::isCloseable() const { return p_w_->isCloseable; }
int GlWindow::width() const { return p_w_->curWidth; }
int GlWindow::height() const { return p_w_->curHeight; }
const std::string& GlWindow::title() const { return p_w_->title; }
const GlWindowTech& GlWindow::info() const { return p_w_->info; }
GlWindowTech& GlWindow::info() { return p_w_->info; }
int GlWindow::mouseX() const { return p_w_->mouseX; }
int GlWindow::mouseY() const { return p_w_->mouseY; }
int GlWindow::mouseXDelta() const { return p_w_->mouseXDelta; }
int GlWindow::mouseYDelta() const { return p_w_->mouseYDelta; }
MouseKeyCodes GlWindow::mouseKeys() const { return p_w_->mouseKeys; }

double GlWindow::fps() const { return p_w_->framesPerSecond; }
#ifdef MO_ENABLE_FPS_TRACE
const ValueBuffer * GlWindow::fpsTrace() const { return p_w_->fpsTrace; }
ValueBuffer * GlWindow::fpsTrace() { return p_w_->fpsTrace; }
#endif

void GlWindow::setCloseable(bool enable) { p_w_->isCloseable = enable; }


long GlWindow::keyToQt(KeyCode k)
{
    switch (k)
    {
        case Key_Backspace       : return Qt::Key_Backspace         ;
        case Key_Tab             : return Qt::Key_Tab               ;
        case Key_Enter           : return Qt::Key_Enter             ;
        case Key_VTab            : return Qt::Key_Backtab           ; // ??
        //case Key_FF 			 : return Qt::Key_FF 			    ;
        case Key_Return		     : return Qt::Key_Return		    ;
        case Key_Escape          : return Qt::Key_Escape            ;
        case Key_Space           : return Qt::Key_Space             ;
        case Key_Excl 	         : return Qt::Key_Exclam            ; // ??
        case Key_DoubleQuote     : return Qt::Key_QuoteDbl          ;
        //case Key_Number	         : return Qt::Key_Number	        ;
        case Key_Dollar	         : return Qt::Key_Dollar	        ;
        case Key_Percent	     : return Qt::Key_Percent	        ;
        case Key_Amp		     : return Qt::Key_Ampersand         ;
        case Key_Quote	         : return Qt::Key_QuoteLeft         ;
        case Key_ParOpen	     : return Qt::Key_ParenLeft;
        case Key_ParClose        : return Qt::Key_ParenRight;
        case Key_Multiply        : return Qt::Key_Asterisk;
        case Key_Add		     : return Qt::Key_Plus;
        case Key_Comma	         : return Qt::Key_Comma	            ;
        case Key_Subtract        : return Qt::Key_Minus;
        case Key_Period	         : return Qt::Key_Period	        ;
        case Key_Divide	         : return Qt::Key_Slash;
        case Key_0               : return Qt::Key_0                 ;
        case Key_1               : return Qt::Key_1                 ;
        case Key_2               : return Qt::Key_2                 ;
        case Key_3               : return Qt::Key_3                 ;
        case Key_4               : return Qt::Key_4                 ;
        case Key_5               : return Qt::Key_5                 ;
        case Key_6               : return Qt::Key_6                 ;
        case Key_7               : return Qt::Key_7                 ;
        case Key_8               : return Qt::Key_8                 ;
        case Key_9               : return Qt::Key_9                 ;
        case Key_Colon	         : return Qt::Key_Colon	            ;
        case Key_Semicolon       : return Qt::Key_Semicolon         ;
        //case Key_LessThan        : return Qt::Key_LessThan          ;
        case Key_Equal	         : return Qt::Key_Equal	            ;
        //case Key_GreaterThan     : return Qt::Key_GreaterThan       ;
        case Key_Question        : return Qt::Key_Question          ;
        case Key_At		         : return Qt::Key_At		        ;
        case Key_BracketOpen     : return Qt::Key_BracketLeft;
        case Key_Backslash       : return Qt::Key_Backslash         ;
        case Key_BracketClose    : return Qt::Key_BracketRight      ;
        //case Key_Caret		     : return Qt::Key_Caret		        ;
        case Key_Underscore	     : return Qt::Key_Underscore	    ;
        //case Key_Accent		     : return Qt::Key_Accent		    ;
        case Key_A               : return Qt::Key_A                 ;
        case Key_B               : return Qt::Key_B                 ;
        case Key_C               : return Qt::Key_C                 ;
        case Key_D               : return Qt::Key_D                 ;
        case Key_E               : return Qt::Key_E                 ;
        case Key_F               : return Qt::Key_F                 ;
        case Key_G               : return Qt::Key_G                 ;
        case Key_H               : return Qt::Key_H                 ;
        case Key_I               : return Qt::Key_I                 ;
        case Key_J               : return Qt::Key_J                 ;
        case Key_K               : return Qt::Key_K                 ;
        case Key_L               : return Qt::Key_L                 ;
        case Key_M               : return Qt::Key_M                 ;
        case Key_N               : return Qt::Key_N                 ;
        case Key_O               : return Qt::Key_O                 ;
        case Key_P               : return Qt::Key_P                 ;
        case Key_Q               : return Qt::Key_Q                 ;
        case Key_R               : return Qt::Key_R                 ;
        case Key_S               : return Qt::Key_S                 ;
        case Key_T               : return Qt::Key_T                 ;
        case Key_U               : return Qt::Key_U                 ;
        case Key_V               : return Qt::Key_V                 ;
        case Key_W               : return Qt::Key_W                 ;
        case Key_X               : return Qt::Key_X                 ;
        case Key_Y               : return Qt::Key_Y                 ;
        case Key_Z               : return Qt::Key_Z                 ;
        case Key_BraceOpen	     : return Qt::Key_BraceLeft;
        case Key_Bar			 : return Qt::Key_Bar			    ;
        case Key_BraceClose	     : return Qt::Key_BraceRight;
        case Key_Tilde		     : return Qt::Key_AsciiTilde        ;
        case Key_Delete          : return Qt::Key_Delete            ;
        case Key_Up              : return Qt::Key_Up                ;
        case Key_Down            : return Qt::Key_Down              ;
        case Key_Left            : return Qt::Key_Left              ;
        case Key_Right           : return Qt::Key_Right             ;
        case Key_PageUp          : return Qt::Key_PageUp            ;
        case Key_PageDown        : return Qt::Key_PageDown          ;
        case Key_Home            : return Qt::Key_Home              ;
        case Key_End             : return Qt::Key_End               ;
        case Key_Insert          : return Qt::Key_Insert            ;
        case Key_Control         : return Qt::Key_Control           ;
        case Key_Alt             : return Qt::Key_Alt               ;
        case Key_Shift           : return Qt::Key_Shift             ;
        case Key_F1              : return Qt::Key_F1                ;
        case Key_F2              : return Qt::Key_F2                ;
        case Key_F3              : return Qt::Key_F3                ;
        case Key_F4              : return Qt::Key_F4                ;
        case Key_F5              : return Qt::Key_F5                ;
        case Key_F6              : return Qt::Key_F6                ;
        case Key_F7              : return Qt::Key_F7                ;
        case Key_F8              : return Qt::Key_F8                ;
        case Key_F9              : return Qt::Key_F9                ;
        case Key_F10             : return Qt::Key_F10               ;
        case Key_F11             : return Qt::Key_F11               ;
        case Key_F12             : return Qt::Key_F12               ;
        case Key_CapsLock        : return Qt::Key_CapsLock          ;
        case Key_NumLock         : return Qt::Key_NumLock           ;
        case Key_Pause           : return Qt::Key_Pause             ;
        //case Key_Super           : return Qt::Key_Super             ;
        case Key_Menu            : return Qt::Key_Menu              ;
        case Key_Meta            : return Qt::Key_Meta              ;
        //case Key_MediaPlayPause  : return Qt::Key_MediaPlayPause    ;
        case Key_MediaStop       : return Qt::Key_MediaStop         ;
        case Key_MediaPrevious   : return Qt::Key_MediaPrevious     ;
        case Key_MediaNext       : return Qt::Key_MediaNext         ;
        default: return -1;
    }
}

long GlWindow::mouseKeyToQt(MouseKeyCode k)
{
    switch (k)
    {
        case MKey_Left: return Qt::LeftButton;
        case MKey_Middle: return Qt::MiddleButton;
        case MKey_Right: return Qt::RightButton;
        default: return -1;
    }
}




} // namespace GL
} // namespace MO
