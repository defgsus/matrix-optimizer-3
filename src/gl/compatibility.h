/** @file compatibility.h

    @brief OpenGL function wrappers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#ifndef MOSRC_GL_COMPATIBILITY_H
#define MOSRC_GL_COMPATIBILITY_H

#include "opengl.h"

namespace MO {
namespace GL {

/** Checks for compatibility of the current driver */
void checkCompatibility();

/** Enables or disables GL_LINE_SMOOTH */
void setLineSmooth(bool enable);

/** Sets the line-width */
void setLineWidth(GLfloat width);



} // namespace GL
} // namespace MO

#endif // COMPATIBILITY_H
