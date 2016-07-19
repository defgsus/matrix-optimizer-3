/** @file lightsettings.cpp

    @brief Scene lighting settings container for use with OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/7/2014</p>
*/

#include "LightSettings.h"
#include "math/constants.h"

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
    directions_.resize(num*3);
    directionParam_.resize(num*3);
    diffuseExp_.resize(num);

    for (uint i=0; i<num; ++i)
    {
        setPosition(i, 10,10,10);
        setColor(i, 1,1,1,1);
        setDiffuseExponent(i, 4);
        setDirection(i, 0,0,-1);
        setDirectionParam(i, 0, 30, 50);
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

void LightSettings::setDiffuseExponent(uint index, Float e)
{
    diffuseExp_[index] = e;
}

void LightSettings::setDirection(uint index, Float nx, Float ny, Float nz)
{
    Float * p = &directions_[index*3];
    *p++ = nx; *p++ = ny; *p++ = nz;
}

void LightSettings::setDirectionParam(uint index, Float mix, Float angle_min, Float angle_max)
{
    Float * p = &directionParam_[index*3];
    *p++ = std::atan((180.f - angle_max) / 340.f * PI);
    *p++ = std::atan((180.f - angle_min) / 340.f * PI);
    *p = mix;
}


} // namespace GL
} // namespace MO

