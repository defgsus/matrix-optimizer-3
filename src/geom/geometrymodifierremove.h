/** @file geometrymodifierremove.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERREMOVE_H
#define GEOMETRYMODIFIERREMOVE_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierRemove : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierRemove)

    // ------------- getter ------------------

    Float getRemoveProbability() const { return prob_; }
    uint getRandomSeed() const { return seed_; }

    // ------------ setter -------------------

    void setRemoveProbability(Float p) { prob_ = p; }
    void setRandomSeed(uint s) { seed_ = s; }

private:

    Float prob_;
    uint seed_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIERREMOVE_H
