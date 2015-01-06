/** @file lightsettings.cpp

    @brief Scene lighting settings container for use with OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/7/2014</p>
*/

#include "lightsettings.h"

namespace MO {
namespace GL {


LightSettings::LightSettings()
    : count_        (0)
{ }

void LightSettings::resize(uint num)
{
    count_ = num;
    positions_.resize(num*3);
    colors_.resize(num*4);
    directions_.resize(num*4);
    directionMix_.resize(num);
    diffuseExp_.resize(num);

    for (uint i=0; i<num; ++i)
    {
        setPosition(i, 10,10,10);
        setColor(i, 1,1,1,1);
        setDirection(i, 0,0,-1, 4);
        setDirectionMix(i, 0);
        setDiffuseExponent(i, 4);
    }
}


void LightSettings::setPosition(uint index, Float x, Float y, Float z)
{
    Float * p = &positions_[index*3];
    *p++ = x; *p++ = y; *p = z;
}


void LightSettings::setColor(uint index, Float r, Float g, Float b, Float d)
{
    Float * p = &colors_[index*4];
    *p++ = r; *p++ = g; *p++ = b; *p = d;
}

void LightSettings::setDirection(uint index, Float nx, Float ny, Float nz, Float exp)
{
    Float * p = &directions_[index*4];
    *p++ = nx; *p++ = ny; *p++ = nz; *p = exp;
}

void LightSettings::setDirectionMix(uint index, Float mix)
{
    directionMix_[index] = mix;
}

void LightSettings::setDiffuseExponent(uint index, Float e)
{
    diffuseExp_[index] = e;
}


} // namespace GL
} // namespace MO

