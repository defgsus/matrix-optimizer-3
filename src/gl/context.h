/** @file gl.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_GL_GL_H
#define MOSRC_GL_GL_H

#include <QOpenGLContext>

class QOpenGLFunctions;

namespace MO {
namespace GL {

class Context : public QOpenGLContext
{
    Q_OBJECT
public:
    explicit Context(QObject * parent);
    ~Context();

    //QOpenGLFunctions * glFuncions() const { return glFunctions_; }

private:
    //QOpenGLFunctions * glFunctions_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_GL_H
