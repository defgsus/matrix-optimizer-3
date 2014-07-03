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

    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ObjectGl)

    virtual Type type() const { return T_OBJECT; }
    bool isGl() const { return true; }

    virtual void setNumberThreads(int num);

    /** Returns the current GL::Context */
    GL::Context * glContext(int thread) { return glContext_[thread]; }
    /** Returns the current GL::Context */
    const GL::Context * glContext(int thread) const { return glContext_[thread]; }

    bool needsInitGl(int thread) const { return needsInitGl_[thread]; }

    virtual void initGl(int thread) = 0;
    virtual void renderGl(int thread, Double time) = 0;

signals:

public slots:

private:

    /** Sets the OpenGL Context */
    void setGlContext_(int thread, GL::Context *);

    void initGl_(int thread);
    void renderGl_(int thread, Double time);

    std::vector<GL::Context*> glContext_;
    bool glFunctionsInitialized_;
    std::vector<int> needsInitGl_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
