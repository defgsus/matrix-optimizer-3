/** @file window.h

    @brief default output window

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GL_WINDOW_H
#define MOSRC_GL_WINDOW_H

#include <QWindow>

#include "opengl_fwd.h"
#include "types/vector.h"

class QTime;

namespace MO {
namespace GL {

/** OpenGL output window.

    This window creates an OpenGL context and sends it off
    to MO::GL::Manager when it first gets shown.

    It can be made fullscreen with F11 (or ALT+F).
*/
class Window : public QWindow
{
    Q_OBJECT
public:
    explicit Window(QScreen * targetScreen = 0);
    ~Window();

    /** Returns the opengl context associated to this window */
    Context * context() const { return context_; }

    /** Returns the messured fps (valid when animating) */
    double messuredFps() const { return fps_; }

    /** Sets the thread identifier for this window/context */
    void setThread(uint thread) { thread_ = thread; }
    /** Returns the thread identifier for this window/context */
    uint threadId() const { return thread_; }

signals:

    /** This will signal a creation of a new Context */
    void contextCreated(uint thread, MO::GL::Context *);

    /** Context is current, please render */
    void renderRequest(uint thread);

    /** Send when the camera matrix changed (in free-camera-mode) */
    void cameraMatrixChanged(const MO::Mat4&);

public slots:

    /** Immediately render on current thread */
    void renderNow();
    /** Puts a render request into the event-loop */
    void renderLater();
    /** Starts consecutively rendering */
    void startAnimation() { animating_ = true; renderLater(); }
    /** Stops rendering consecutively */
    void stopAnimation() { animating_ = false; }

protected:
    bool event(QEvent *);
    void exposeEvent(QExposeEvent *);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:

    void render_();

    Context * context_;

    uint thread_;

    bool updatePending_,
        animating_;

    QTime * messure_;

    double fps_;

    bool isFreeCamera_;
    GEOM::FreeCamera * cameraControl_;

    QPoint lastMousePos_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_WINDOW_H
