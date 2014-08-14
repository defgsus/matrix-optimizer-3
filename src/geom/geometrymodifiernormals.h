/** @file geometrymodifiernormals.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERNORMALS_H
#define GEOMETRYMODIFIERNORMALS_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierNormals : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierNormals)

    // ------------- getter ------------------

    bool getCalcNormals() const { return calc_; }
    bool getInvertNormals() const { return invert_; }

    // ------------ setter -------------------

    void setCalcNormals(bool doo) { calc_ = doo; }
    void setInvertNormals(bool doo) { invert_ = doo; }

private:

    bool calc_, invert_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIERNORMALS_H
