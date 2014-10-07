/** @file geometrymodifierduplicate.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.10.2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERDUPLICATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERDUPLICATE_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierDuplicate : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierDuplicate)

    // ------------- getter ------------------

    uint duplicatesX() const { return numX_; }
    uint duplicatesY() const { return numY_; }
    uint duplicatesZ() const { return numZ_; }

    const QString& getEquation() const { return equ_; }

    // ------------ setter -------------------

    void setEquation(const QString& e) { equ_ = e; }

    void setDuplicatesX(uint num) { numX_ = num; }
    void setDuplicatesY(uint num) { numY_ = num; }
    void setDuplicatesZ(uint num) { numZ_ = num; }

private:

    QString equ_;
    uint numX_, numY_, numZ_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERDUPLICATE_H
