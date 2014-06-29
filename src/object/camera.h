/** @file camera.h

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_CAMERA_H
#define MOSRC_OBJECT_CAMERA_H

#include "objectgl.h"

namespace MO {

class Camera : public ObjectGl
{
    Q_OBJECT
public:
    explicit Camera(QObject *parent = 0);

    virtual const QString& className() const { static QString s("Camera"); return s; }

    virtual bool isCamera() const { return true; }

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_OBJECT_CAMERA_H
