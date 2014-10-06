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

    const QString& getEquationX() const { return equX_; }
    const QString& getEquationY() const { return equY_; }
    const QString& getEquationZ() const { return equZ_; }

    // ------------ setter -------------------

    void setEquationX(const QString& e) { equX_ = e; }
    void setEquationY(const QString& e) { equY_ = e; }
    void setEquationZ(const QString& e) { equZ_ = e; }

    void setDuplicatesX(uint num) { numX_ = num; }
    void setDuplicatesY(uint num) { numY_ = num; }
    void setDuplicatesZ(uint num) { numZ_ = num; }

private:

    QString equX_, equY_, equZ_;
    uint numX_, numY_, numZ_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERDUPLICATE_H
