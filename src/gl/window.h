/** @file window.h

    @brief default output window

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GL_WINDOW_H
#define MOSRC_GL_WINDOW_H

#include <QWindow>

class QOpenGLFramebufferObject;

namespace MO {
namespace GL {

class Context;

class Window : public QWindow
{
    Q_OBJECT
public:
    explicit Window(QScreen * targetScreen = 0);

    bool isFullScreen() const { return isFullScreen_; }
    void setFullScreen(bool fs);

    Context * context() const { return context_; }
    void setContext(Context*);
    void setFramebuffer(QOpenGLFramebufferObject * frameBuffer);

protected:
    /** For paint event */
    bool event(QEvent *);
    void exposeEvent(QExposeEvent *);
    /** To set Visibility(FullScreen) */
    void showEvent(QShowEvent *);

private:

    void render_();

    Context * context_;
    QOpenGLFramebufferObject * frameBuffer_;

    bool isFullScreen_;
         //updatePending_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_WINDOW_H
