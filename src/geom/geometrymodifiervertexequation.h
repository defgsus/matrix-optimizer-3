/** @file geometrymodifiervertexequation.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERVERTEXEQUATION_H
#define GEOMETRYMODIFIERVERTEXEQUATION_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierVertexEquation : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierVertexEquation)

    // ------------- getter ------------------

    const QString& getEquation() const { return equ_; }

    // ------------ setter -------------------

    void setEquation(const QString& e) { equ_ = e; }

private:

    QString equ_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIERVERTEXEQUATION_H
