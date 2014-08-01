/** @file shadersource.cpp

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <QFile>

#include "shadersource.h"
#include "io/log.h"
#include "io/error.h"

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

void ShaderSource::loadFragmentSource(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load fragment source from '" << filename << "'\n"
                    << f.errorString());

    frag_ = f.readAll();
}


void ShaderSource::loadVertexSource(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load vertex source from '" << filename << "'\n"
                    << f.errorString());

    vert_ = f.readAll();
}

void ShaderSource::loadDefaultSource()
{
    loadVertexSource(":/shader/default.vert");
    loadFragmentSource(":/shader/default.frag");
}


} // namespace GL
} // namespace MO
