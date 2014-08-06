/** @file LightSettings.h

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
class Scene;
namespace GL {


/** Contains all lightsources in scene read to send to shader */
class LightSettings
{
public:

    /** Returns the number of light sources */
    uint count() const { return positions_.size(); }

    /** Returns count() sequential x,y,z pairs */
    const Float * positions() const { return &positions_[0]; }

    /** Returns count() sequential r,g,b pairs */
    const Float * colors() const { return &colors_[0]; }

private:
    friend class MO::Scene;

    void resize_(uint num) { positions_.resize(num); colors_.resize(num); }

    std::vector<Float> positions_, colors_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_LIGHTSETTINGS_H
