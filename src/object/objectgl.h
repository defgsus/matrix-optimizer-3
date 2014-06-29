/** @file objectgl.h

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTGL_H
#define MOSRC_OBJECT_OBJECTGL_H

#include "object3d.h"

namespace MO {

class ObjectGl : public Object3d
{
    Q_OBJECT
public:
    explicit ObjectGl(QObject *parent = 0);

    bool isGl() const { return true; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTGL_H
