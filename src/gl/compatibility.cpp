/** @file compatibility.cpp

    @brief OpenGL function wrappers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include "compatibility.h"
#include "io/log.h"

namespace MO {
namespace GL {

namespace
{
    static bool
        enableLineSmooth_ = false,
        enableLineWidth_ = false;
}

void checkCompatibility()
{
    const QString
            vendor = (const char *)glGetString(GL_VENDOR),
            version = (const char *)glGetString(GL_VERSION),
            renderer = (const char *)glGetString(GL_RENDERER);

    MO_DEBUG_GL(     "vendor   " << vendor
                << "\nversion  " << version
                << "\nrenderer " << renderer);

    bool isMesa = renderer.contains("Mesa", Qt::CaseInsensitive);

    // sorry mesa guys, but...
    enableLineSmooth_ = !isMesa;

    // test line width
    glLineWidth(2);
    enableLineWidth_ = (glGetError() == 0);
    glLineWidth(1);
}

void setLineSmooth(bool enable)
{
    if (!enableLineSmooth_)
        return;

    if (enable)
        MO_CHECK_GL( glEnable(GL_LINE_SMOOTH) )
    else
        MO_CHECK_GL( glDisable(GL_LINE_SMOOTH) );
}

void setLineWidth(GLfloat width)
{
    if (!enableLineWidth_)
        return;

    MO_CHECK_GL( glLineWidth(width) );
}


} // namespace GL
} // namespace MO
