/** @file geometrymodifiertranslate.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERTRANSLATE_H
#define MOSRC_GEOM_GEOMETRYMODIFIERTRANSLATE_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {

class GeometryModifierTranslate : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierTranslate)

    // ------------- getter ------------------

    Float getTranslationX() const { return x_; }
    Float getTranslationY() const { return y_; }
    Float getTranslationZ() const { return z_; }

    // ------------ setter -------------------

    void setTranslationX(Float s) { x_ = s; }
    void setTranslationY(Float s) { y_ = s; }
    void setTranslationZ(Float s) { z_ = s; }

private:

    Float x_, y_, z_;
};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYMODIFIERTRANSLATE_H
