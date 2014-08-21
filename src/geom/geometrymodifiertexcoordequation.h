/** @file geometrymodifiertexcoordequation.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/22/2014</p>
*/

#ifndef GEOMETRYMODIFIERTEXCOORDEQUATION_H
#define GEOMETRYMODIFIERTEXCOORDEQUATION_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierTexCoordEquation : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierTexCoordEquation)

    // ------------- getter ------------------

    const QString& getEquationS() const { return equS_; }
    const QString& getEquationT() const { return equT_; }

    // ------------ setter -------------------

    void setEquationS(const QString& e) { equS_ = e; }
    void setEquationT(const QString& e) { equT_ = e; }
private:

    QString equS_, equT_;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYMODIFIERTEXCOORDEQUATION_H
