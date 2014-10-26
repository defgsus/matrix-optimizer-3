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

class Context : public QObject
{
    Q_OBJECT
public:
    explicit Context(QObject * parent);
    ~Context();

    QOpenGLContext * qcontext() const { return qcontext_; }

    const QSize& size() const { return size_; }
    void setSize(const QSize& size) { size_ = size; }
    void setSurface(QSurface* s) { surface_ = s; }

    bool isValid() const;

    bool makeCurrent();

private:

    QSize size_;
    QOpenGLContext * qcontext_;
    QSurface * surface_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CONTEXT_H
