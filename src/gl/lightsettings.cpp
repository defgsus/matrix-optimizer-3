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
    colors_.resize(num*3);
}


void LightSettings::setPosition(uint index, Float x, Float y, Float z)
{
    Float * p = &positions_[index*3];
    *p++ = x; *p++ = y; *p = z;
}


void LightSettings::setColor(uint index, Float r, Float g, Float b)
{
    Float * p = &colors_[index*3];
    *p++ = r; *p++ = g; *p = b;
}


} // namespace GL
} // namespace MO

