/** @file geometrymodifiertexcoords.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/21/2014</p>
*/

#ifndef GEOMETRYMODIFIERTEXCOORDS_H
#define GEOMETRYMODIFIERTEXCOORDS_H


#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierTexCoords : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierTexCoords)

    // ------------- getter ------------------

    Float getOffsetX() const { return offsetX_; }
    Float getOffsetY() const { return offsetY_; }
    Float getScaleX() const { return scaleX_; }
    Float getScaleY() const { return scaleY_; }
    bool getInvertX() const { return invertX_; }
    bool getInvertY() const { return invertY_; }

    // ------------ setter -------------------

    void setOffsetX(Float s) { offsetX_ = s; }
    void setOffsetY(Float s) { offsetY_ = s; }
    void setScaleX(Float s) { scaleX_ = s; }
    void setScaleY(Float s) { scaleY_ = s; }
    void setInvertX(bool i) { invertX_ = i; }
    void setInvertY(bool i) { invertY_ = i; }

private:

    Float offsetX_, offsetY_, scaleX_, scaleY_;
    bool invertX_, invertY_;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYMODIFIERTEXCOORDS_H
