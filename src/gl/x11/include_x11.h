/** @file

    @brief Wrapper to include X11 and clean up the namespace afterwards

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/20/2016</p>
*/

#ifndef MOSRC_GL_X11_INCLUDE_X11_H
#define MOSRC_GL_X11_INCLUDE_X11_H

// glbinding must be first
#include "gl/opengl.h"
// pollute the namespace
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
// undef opengl defines
#include "gl/opengl_undef.h"

// -- undef X11 defines --

/** Namespace for typed wrappers of previous X11 defines */
namespace XX
{
    const long X_None             = None            ;
    const long X_NoSymbol         = NoSymbol        ;
    const long X_KeyPress         = KeyPress        ;
    const long X_KeyRelease       = KeyRelease      ;
    const long X_ButtonPress      = ButtonPress     ;
    const long X_ButtonRelease    = ButtonRelease   ;
    const long X_MotionNotify     = MotionNotify    ;
    const long X_EnterNotify      = EnterNotify     ;
    const long X_LeaveNotify      = LeaveNotify     ;
    const long X_KeymapNotify     = KeymapNotify    ;
    const long X_VisibilityNotify = VisibilityNotify;
    const long X_CreateNotify     = CreateNotify    ;
    const long X_DestroyNotify    = DestroyNotify   ;
    const long X_UnmapNotify      = UnmapNotify     ;
    const long X_MapNotify        = MapNotify       ;
    const long X_MapRequest       = MapRequest      ;
    const long X_ReparentNotify   = ReparentNotify  ;
    const long X_ConfigureNotify  = ConfigureNotify ;
    const long X_ConfigureRequest = ConfigureRequest;
    const long X_GravityNotify    = GravityNotify   ;
    const long X_ResizeRequest    = ResizeRequest   ;
    const long X_CirculateNotify  = CirculateNotify ;
    const long X_CirculateRequest = CirculateRequest;
    const long X_PropertyNotify   = PropertyNotify  ;
    const long X_SelectionClear   = SelectionClear  ;
    const long X_SelectionRequest = SelectionRequest;
    const long X_SelectionNotify  = SelectionNotify ;
    const long X_ColormapNotify   = ColormapNotify  ;
    const long X_ClientMessage    = ClientMessage   ;
    const long X_MappingNotify    = MappingNotify   ;
    const long X_ShiftMask        = ShiftMask       ;
    const long X_LockMask         = LockMask        ;
    const long X_Button1          = Button1         ;
    const long X_Button2          = Button2         ;
    const long X_Button3          = Button3         ;
    const long X_Button4          = Button4         ;
    const long X_Button5          = Button5         ;

} // namespace XX

#undef FontChange
#undef CursorShape
#undef Status
#undef Bool
#undef None
#undef KeyPress
#undef KeyRelease
#undef ButtonPress
#undef ButtonRelease
#undef MotionNotify
#undef EnterNotify
#undef LeaveNotify
#undef FocusIn
#undef FocusOut
#undef KeymapNotify
#undef Expose
#undef GraphicsExpose
#undef NoExpose
#undef VisibilityNotify
#undef CreateNotify
#undef DestroyNotify
#undef UnmapNotify
#undef MapNotify
#undef MapRequest
#undef ReparentNotify
#undef ConfigureNotify
#undef ConfigureRequest
#undef GravityNotify
#undef ResizeRequest
#undef CirculateNotify
#undef CirculateRequest
#undef PropertyNotify
#undef SelectionClear
#undef SelectionRequest
#undef SelectionNotify
#undef ColormapNotify
#undef ClientMessage
#undef MappingNotify
#undef GenericEvent
#undef ShiftMask
#undef LockMask
#undef LASTEvent
#undef Button1
#undef Button2
#undef Button3
#undef Button4
#undef Button5

#endif // MOSRC_GL_X11_INCLUDE_X11_H

