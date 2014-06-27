/** @file object3d.h

    @brief positional Object base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECT3D_H
#define MOSRC_OBJECT_OBJECT3D_H

#include "object.h"
#include "types/vector.h"

namespace MO {


class Object3d : public Object
{
    Q_OBJECT
public:
    explicit Object3d(QObject *parent = 0);

    // ------------------- getter -------------------

    bool is3d() const { return true; }

    /** Returns the transformation matrix of this object */
    const Mat4& transformation() const { return transformation_; }

    Vec3 position() const
        { return Vec3(transformation_[3][0], transformation_[3][1], transformation_[3][2]); }

signals:

public slots:

private:

    Mat4 transformation_;
};


} // namespace MO

#endif // MOSRC_OBJECT_OBJECT3D_H
