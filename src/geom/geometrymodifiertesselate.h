/** @file geometrymodifiertesselate.h

    @brief Tesselates a Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERTESSELATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERTESSELATE_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierTesselate : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierTesselate)

    // ------------- getter ------------------

    Float getTesselationLevel() const { return level_; }
    Float getMinArea() const { return minArea_; }
    Float getMinLength() const { return minLength_; }

    // ------------ setter -------------------

    void setTesselationLevel(uint t) { level_ = std::max((uint)1, t); }
    void setMinArea(Float minArea) { minArea_ = minArea; }
    void setMinLength(Float minLength) { minLength_ = minLength; }

private:

    uint level_;
    Float minArea_, minLength_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERTESSELATE_H
