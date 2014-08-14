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
    GeometryModifierTesselate();

    // ------------- getter ------------------

    Float getTesselationLevel() const { return level_; }

    // ------------ setter -------------------

    void setTesselationLevel(uint t) { level_ = std::max((uint)1, t); }

    // ----------- virtual interface ---------

    virtual void serialize(IO::DataStream& io) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream& io) Q_DECL_OVERRIDE;

    virtual GeometryModifierTesselate * cloneClass() const Q_DECL_OVERRIDE
                        { return new GeometryModifierTesselate(); }

    virtual void execute(Geometry * g) Q_DECL_OVERRIDE;

private:

    uint level_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERTESSELATE_H
