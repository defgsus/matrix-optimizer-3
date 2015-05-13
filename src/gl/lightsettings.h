/** @file lightsettings.h

    @brief Scene lighting settings container for use with OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/7/2014</p>
*/

#ifndef MOSRC_GL_LIGHTSETTINGS_H
#define MOSRC_GL_LIGHTSETTINGS_H

#include <vector>

#include "types/float.h"

namespace MO {
namespace GL {


/** Contains all lightsources in scene, ready to send to shader */
class LightSettings
{
public:

    LightSettings();

    // --------------- getter ---------------

    /** Returns the number of light sources */
    uint count() const { return count_; }

    /** Returns count() sequential x,y,z pairs */
    const Float * positions() const { return &positions_[0]; }

    /** Returns count() sequential r,g,b,d pairs.
        d is the distance attenuation factor. */
    const Float * colors() const { return &colors_[0]; }

    /** Returns count() sequential floats */
    const Float * diffuseExponents() const { return &diffuseExp_[0]; }

    /** Returns count() sequential x,y,z pairs.
        The direction vector is normalized. */
    const Float * directions() const { return &directions_[0]; }

    /** Returns count() sequential min,max,mix pairs for setting
        directional light parameters */
    const Float * directionParam() const { return &directionParam_[0]; }

    // --------------- setter ---------------

    /** Reserves space for the @p num light sources */
    void resize(uint num);

    void setPosition(uint index, Float x, Float y, Float z);
    void setColor(uint index, Float r, Float g, Float b, Float d = 0.0);
    void setDiffuseExponent(uint index, Float exp);
    void setDirection(uint index, Float nx, Float ny, Float nz);
    void setDirectionParam(uint index, Float mix, Float angle_min, Float angle_max);

private:

    std::vector<Float>
        positions_,
        colors_,
        directions_,
        directionParam_,
        diffuseExp_;

    uint count_;
};


} // namespace GL
} // namespace MO

#endif // MOSRC_GL_LIGHTSETTINGS_H
