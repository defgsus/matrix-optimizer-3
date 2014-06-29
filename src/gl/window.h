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
    ~Window();

    Context * context() const { return context_; }
//    void setFramebuffer(QOpenGLFramebufferObject * frameBuffer);

signals:

    /** This will signal a creation of a new Context */
    void contextCreated(MO::GL::Context *);

    /** Context is current, please render */
    void renderRequest();

public slots:

    void renderNow();
    void renderLater();

protected:
    bool event(QEvent *);
    void exposeEvent(QExposeEvent *);

private:

    void render_();

    Context * context_;
//    QOpenGLFramebufferObject * frameBuffer_;

    bool updatePending_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_WINDOW_H
