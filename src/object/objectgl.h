/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object.h"
#include "gl/opengl.h"

namespace MO {
namespace GL { class Context; class Drawable; }

class ObjectGl : public Object
{
    Q_OBJECT

    // for gl context handling
    friend class Scene;

public:

    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ObjectGl)

    virtual Type type() const Q_DECL_OVERRIDE { return T_OBJECT; }
    bool isGl() const Q_DECL_OVERRIDE { return true; }

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    /** Returns the current GL::Context */
    GL::Context * glContext(uint thread) { return glContext_[thread]; }
    /** Returns the current GL::Context */
    const GL::Context * glContext(uint thread) const { return glContext_[thread]; }

    bool needsInitGl(uint thread) const { return needsInitGl_[thread]; }
    bool isGlInitialized(uint thread) const { return isGlInitialized_[thread]; }

    virtual void initGl(uint thread) = 0;
    virtual void releaseGl(uint thread) = 0;

    virtual void renderGl(const GL::RenderSettings& rs, uint thread, Double time) = 0;

    /** Tell scene that this object wants to be painted */
    void requestRender();

    /** Requests a releaseGl() - initGl() sequence */
    void requestReinitGl();

signals:

public slots:

private:

    /** Sets the OpenGL Context */
    void setGlContext_(uint thread, GL::Context *);

    void initGl_(uint thread);
    void releaseGl_(uint thread);
    void renderGl_(const GL::RenderSettings& rs, uint thread, Double time);

    std::vector<GL::Context*> glContext_;
    std::vector<int> needsInitGl_, isGlInitialized_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
