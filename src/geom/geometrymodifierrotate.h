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
    GeometryModifierRotate();

    // ------------- getter ------------------

    Float getRotateAngle() const { return angle_; }
    Float getRotateX() const { return x_; }
    Float getRotateY() const { return y_; }
    Float getRotateZ() const { return z_; }

    // ------------ setter -------------------

    void setRotateAngle(Float s) { angle_ = s; }
    void setRotateX(Float s) { x_ = s; }
    void setRotateY(Float s) { y_ = s; }
    void setRotateZ(Float s) { z_ = s; }

    // ----------- virtual interface ---------

    virtual void serialize(IO::DataStream& io) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream& io) Q_DECL_OVERRIDE;

    virtual GeometryModifierRotate * cloneClass() const Q_DECL_OVERRIDE
                        { return new GeometryModifierRotate(); }

    virtual void execute(Geometry * g) Q_DECL_OVERRIDE;

private:

    Float angle_, x_, y_, z_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERROTATE_H
