/** @file geometrymodifiervertexgroup.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERVERTEXGROUP_H
#define GEOMETRYMODIFIERVERTEXGROUP_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierVertexGroup : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierVertexGroup)

    // ------------- getter ------------------

    bool getShared() const { return share_; }
    Float getThreshold() const { return threshold_; }

    // ------------ setter -------------------

    void setShared(bool s) { share_ = s; }
    void setThreshold(Float t) { threshold_ = t; }

private:

    bool share_;
    Float threshold_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIERVERTEXGROUP_H
