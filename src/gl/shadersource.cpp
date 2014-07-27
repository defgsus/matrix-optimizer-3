/** @file shadersource.cpp

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include "shadersource.h"

namespace MO {
namespace GL {

ShaderSource::ShaderSource()
    : unProj_   ("u_projection"),
      unView_   ("u_view"),
      anPos_    ("a_pos")
{
}


} // namespace GL
} // namespace MO
