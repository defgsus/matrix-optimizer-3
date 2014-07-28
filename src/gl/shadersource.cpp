/** @file shadersource.cpp

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <QFile>

#include "shadersource.h"
#include "io/log.h"
namespace MO {
namespace GL {

ShaderSource::ShaderSource()
    : unProj_   ("u_projection"),
      unView_   ("u_view"),
      anPos_    ("a_position"),
      anCol_    ("a_color"),
      anNorm_   ("a_normal"),
      anTexCoord_("a_texCoord")
{
}

void ShaderSource::setDefaultSource()
{
    {
        QFile f(":/shader/default.vert");
        f.open(QIODevice::ReadOnly);
        vert_ = f.readAll();
    }
    {
        QFile f(":/shader/default.frag");
        f.open(QIODevice::ReadOnly);
        frag_ = f.readAll();
    }
}


} // namespace GL
} // namespace MO
