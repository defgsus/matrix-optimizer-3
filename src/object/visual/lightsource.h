/** @file lightsource.h

    @brief A light source Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_LIGHTSOURCE_H
#define MOSRC_OBJECT_VISUAL_LIGHTSOURCE_H

#include "object/object.h"

namespace MO {

/** Scene object encapsulating light source settings */
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
    Vec4 lightColor(Double time, uint thread) const;

    /** fill the settings container with current values */
    void getLightSettings(GL::LightSettings *, uint index, Double time, uint thread);

signals:

public slots:

private:

    ParameterFloat *all_, *r_, *g_, *b_,
            *dist_, *directional_,
            *directionAngle_, *directionRange_,
            *diffuseExp_;

};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_LIGHTSOURCE_H
