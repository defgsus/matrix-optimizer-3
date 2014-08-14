/** @file geometrymodifiernormalize.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERNORMALIZE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERNORMALIZE_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierNormalize : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierNormalize)

    // ------------- getter ------------------

    Float getNormalization() const { return n_; }

    // ------------ setter -------------------

    void setNormalization(Float n) { n_ = n; }

private:

    Float n_;
};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYMODIFIERNORMALIZE_H
