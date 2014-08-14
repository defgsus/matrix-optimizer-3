/** @file geometrymodifierscale.h

    @brief scales a geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERSCALE_H
#define GEOMETRYMODIFIERSCALE_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierScale : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierScale)

    // ------------- getter ------------------

    Float getScaleAll() const { return all_; }
    Float getScaleX() const { return x_; }
    Float getScaleY() const { return y_; }
    Float getScaleZ() const { return z_; }

    // ------------ setter -------------------

    void setScaleAll(Float s) { all_ = s; }
    void setScaleX(Float s) { x_ = s; }
    void setScaleY(Float s) { y_ = s; }
    void setScaleZ(Float s) { z_ = s; }

private:

    Float all_, x_, y_, z_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIERSCALE_H
