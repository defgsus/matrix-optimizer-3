/** @file rendersettings.cpp

    @brief RenderSettings including all infos, passed to objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#include "rendersettings.h"

namespace MO {
namespace GL {


RenderSettings::RenderSettings()
    : camSpace_     (0),
      lightSettings_(0)
{
}


} // namespace GL
} // namespace MO
