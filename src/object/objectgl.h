/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object.h"
#include "gl/openglfunctions.h"

namespace MO {
namespace GL { class Context; }

class ObjectGl : public Object,
                 protected MO_QOPENGL_FUNCTIONS_CLASS
{
    Q_OBJECT

    friend Scene;

public:

    explicit ObjectGl(QObject *parent = 0);

    virtual Type type() const { return T_OBJECT; }
    bool isGl() const { return true; }

    /** Returns the current GL::Context */
    GL::Context * glContext() { return glContext_; }
    /** Returns the current GL::Context */
    const GL::Context * glContext() const { return glContext_; }

    bool needsInitGl() const { return needsInitGl_; }

    virtual void initGl() = 0;
    virtual void renderGl(Double time) = 0;

signals:

public slots:

private:

    /** Sets the OpenGL Context */
    void setGlContext_(GL::Context *);

    void initGl_();
    void renderGl_(Double time);

    GL::Context * glContext_;
    bool glFunctionsInitialized_,
         needsInitGl_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
