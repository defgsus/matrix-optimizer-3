
/** @file
  * GL CONTEXT MANEGMENT
  *
  * @author def.gsus-
  * @version 2011/06/21 v0.1
  * @version 2011/06/22 v0.11 started x11 part
  * @version 2011/11/10 port to MCW tools
  *
  * @code
  * // ---- basic usage -------------
  *
  * GL::Window win(320,200);
  *	GL::Context gl(&win);
  *
  * while (win->update())
  * {
  *    // gl.makeCurrent(); // was also done in instantiation of gl
  *
  *    ... gl stuff ...
  *
  *    win.swapBuffers();
  *	   // ^ IMPORTANT! ḾUST NOT FORGET! or system can get locked
  * }
  * @endcode
  *
  *
  */
#ifndef MOSRC_GL_GLCONTEXT_H_INCLUDED
#define MOSRC_GL_GLCONTEXT_H_INCLUDED

#include <vector>

namespace MO {
namespace GL {

class GlWindow;
struct GlContextTech;

/** This class handles an openGl drawing context
    and wraps the SYSTEM SPECIFIC stuff

    <p>This is independent of Qt to allow for efficient low-level access.</p>

    <p>currently supports as source: <br>
    GL::GlWindow : the device context from the system window is
		used to create the gl context.
	</p>
  */
class GlContext
{
	public:

    /** Swap interval setting */
    enum Sync
    {
        /** Don't sync to vertical retrace */
        SYNC_NONE = 0,
        /** Alway sync to screen refresh rate */
        SYNC_FIXED = 1,
        /** Sync to screen refresh rate if possible */
        SYNC_ADAPTIVE = -1
    };


    // ------------------- ctor -------------------

    /** Creates a context from a window, if @p win != NULL.
        Otherwise use create() later. */
    GlContext(GlWindow *win = 0, int major = 3, int minor = 2);

    /** Releases the context if necessary */
    virtual ~GlContext();

    // ------------------ getter ------------------

    /** Return true if this context is created properly */
    bool isOk() const;

    /** Resolution of the context (usually the size of the window) */
    int width() const;
    /** Resolution of the context (usually the size of the window) */
    int height() const;

    /** Returns currently set vertical synchronisation setting */
    Sync syncMode() const;

    /** Returns a struct with system sepecific variables and handles. */
    const GlContextTech& info() const;

    /** Return the maximum swapgroup */
    unsigned getMaxSwapGroups() const;

    // ------------------ setter ------------------

    /** Creates an OpenGL context for a window, and makes it current.
        Any previous context is discarded.
        Automatically loads the OpenGL functions. */
    void create(GlWindow* win, int major, int minor);

    /** Sets the synchronisation mode. Default is SYNC_FIXED */
    void setSyncMode(Sync mode);

    /** Releases the handles and stuff.
        Ignored if context is not initialized. */
    void release();

    /** Makes this rendering context current to the associated window. */
    void makeCurrent();

    /** Makes this rendering context current to the given window. */
    void makeCurrent(GlWindow * win);

    /** Sets the OpenGL viewport to the current window size */
    void setViewport();

private:

    struct Private;
    Private* p_;

    /** disable copy */	GlContext(const GlContext&) = delete;
    /** disable copy */	void operator=(const GlContext&) = delete;
};




} // namespace GL
} // namespace MO

#endif // MOSRC_GL_GLCONTEXT_H_INCLUDED
