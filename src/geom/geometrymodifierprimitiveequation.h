/** @file geometrymodifierprimitiveequation.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIERPRIMITIVEEQUATION_H
#define GEOMETRYMODIFIERPRIMITIVEEQUATION_H



#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierPrimitiveEquation : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierPrimitiveEquation)

    // ------------- getter ------------------

    const QString& getEquationX() const { return equX_; }
    const QString& getEquationY() const { return equY_; }
    const QString& getEquationZ() const { return equZ_; }

    // ------------ setter -------------------

    void setEquationX(const QString& e) { equX_ = e; }
    void setEquationY(const QString& e) { equY_ = e; }
    void setEquationZ(const QString& e) { equZ_ = e; }
private:

    QString equX_, equY_, equZ_;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYMODIFIERPRIMITIVEEQUATION_H
