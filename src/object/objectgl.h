/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object3d.h"
#include "gl/openglfunctions.h"

namespace MO {
namespace GL { class Context; }

class ObjectGl : public Object3d,
                 protected MO_QOPENGL_FUNCTIONS_CLASS
{
    Q_OBJECT

    friend Scene;

public:

    explicit ObjectGl(QObject *parent = 0);

    bool isGl() const { return true; }

    /** Returns the current GL::Context */
    GL::Context * glContext() { return glContext_; }
    /** Returns the current GL::Context */
    const GL::Context * glContext() const { return glContext_; }

    virtual void render() = 0;

signals:

public slots:

private:

    /** Sets the OpenGL Context */
    void setGlContext_(GL::Context *);

    void render_();

    GL::Context * glContext_;
    bool glFunctionsInitialized_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
