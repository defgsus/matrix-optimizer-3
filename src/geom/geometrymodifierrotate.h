/** @file geometrymodifierrotate.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERROTATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERROTATE_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierRotate : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierRotate)

    // ------------- getter ------------------

    Float getRotationAngle() const { return angle_; }
    Float getRotationX() const { return x_; }
    Float getRotationY() const { return y_; }
    Float getRotationZ() const { return z_; }

    // ------------ setter -------------------

    void setRotationAngle(Float s) { angle_ = s; }
    void setRotationX(Float s) { x_ = s; }
    void setRotationY(Float s) { y_ = s; }
    void setRotationZ(Float s) { z_ = s; }

private:

    Float angle_, x_, y_, z_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERROTATE_H
