/** @file context.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GL_CONTEXT_H
#define MOSRC_GL_CONTEXT_H

#include <QObject>
#include <QSize>

class QOpenGLContext;
class QSurface;

namespace MO {
namespace GL {

class GlContext;
class GlWindow;

/** Wrapper for a QOpenGLContext or GlContext with associated size */
class Context
{
public:
    /** Creates a QOpenGLContext */
    explicit Context();
    /** Creates a GlContext */
    explicit Context(GlWindow*);

    ~Context();

    GlContext* glContext() const { return glContext_; }
    QOpenGLContext * qcontext() const { return qcontext_; }

    const QSize& size() const { return size_; }
    void setSize(const QSize& size) { size_ = size; }

    bool isValid() const;

    bool makeCurrent();
    bool swapBuffers();

    void setSurface(QSurface* s) { surface_ = s; }

protected:

    QSize size_;
    GlContext* glContext_;
    GlWindow* glWindow_;
    QOpenGLContext * qcontext_;
    QSurface * surface_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CONTEXT_H
