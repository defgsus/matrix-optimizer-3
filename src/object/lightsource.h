/** @file light.h

    @brief A light source

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_OBJECT_LIGHTSOURCE_H
#define MOSRC_OBJECT_LIGHTSOURCE_H

#include "object.h"

namespace MO {

class LightSource : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(LightSource);

    virtual Type type() const Q_DECL_OVERRIDE { return T_LIGHTSOURCE; }
    virtual bool isLightSource() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    /** Returns the color at given time.
        The fourth component is the distance attenuation factor. */
    Vec4 lightColor(uint thread, Double time) const;

    /** Returns the direction normal of the lightsource (xyz)
        and the exponent in w.
        The object transformation must be calculated already! */
    Vec4 lightDirection(uint thread, Double time) const;

    /** Returns the float [0,1] to mix between omni-directional and directional */
    Float lightDirectionalMix(uint thread, Double time) const;

    Float diffuseExponent(uint thread, Double time) const;

signals:

public slots:

private:

    ParameterFloat *all_, *r_, *g_, *b_,
            *dist_, *directional_, *directionExp_,
            *diffuseExp_;

};

} // namespace MO

#endif // MOSRC_OBJECT_LIGHTSOURCE_H
