/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object3d.h"

namespace MO {
namespace GL { class Context; }

class ObjectGl : public Object3d
{
    Q_OBJECT
public:
    explicit ObjectGl(QObject *parent = 0);

    bool isGl() const { return true; }

    void setGlContext(GL::Context *);
    GL::Context * glContext() { return glContext_; }
    const GL::Context * glContext() const { return glContext_; }

signals:

public slots:

private:

    GL::Context * glContext_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
