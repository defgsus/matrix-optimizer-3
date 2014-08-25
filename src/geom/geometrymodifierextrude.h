/** @file geometrymodifierextrude.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef GEOMETRYMODIFIEREXTRUDE_H
#define GEOMETRYMODIFIEREXTRUDE_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierExtrude : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierExtrude)

    // ------------- getter ------------------

    Float getConstant() const { return constant_; }
    Float getFactor() const { return factor_; }
    bool getDoOuterFaces() const { return doOuterFaces_; }
    bool getDoRecognizeEdges() const { return doRecogEdges_; }

    // ------------ setter -------------------

    void setConstant(Float s) { constant_ = s; }
    void setFactor(Float s) { factor_ = s; }
    void setDoOuterFaces(bool enable) { doOuterFaces_ = enable; }
    void setDoRecognizeEdges(bool enable) { doRecogEdges_ = enable; }

private:

    Float constant_, factor_;
    bool doOuterFaces_, doRecogEdges_;
};


} // namespace GEOM
} // namespace MO


#endif // GEOMETRYMODIFIEREXTRUDE_H
