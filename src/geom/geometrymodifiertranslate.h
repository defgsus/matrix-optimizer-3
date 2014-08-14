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
    GeometryModifierTranslate();

    // ------------- getter ------------------

    Float getTranslateX() const { return x_; }
    Float getTranslateY() const { return y_; }
    Float getTranslateZ() const { return z_; }

    // ------------ setter -------------------

    void setTranslateX(Float s) { x_ = s; }
    void setTranslateY(Float s) { y_ = s; }
    void setTranslateZ(Float s) { z_ = s; }

    // ----------- virtual interface ---------

    virtual void serialize(IO::DataStream& io) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream& io) Q_DECL_OVERRIDE;

    virtual GeometryModifierTranslate * cloneClass() const Q_DECL_OVERRIDE
                        { return new GeometryModifierTranslate(); }

    virtual void execute(Geometry * g) Q_DECL_OVERRIDE;

private:

    Float x_, y_, z_;
};


} // namespace GEOM
} // namespace MO


#endif // MOSRC_GEOM_GEOMETRYMODIFIERTRANSLATE_H
