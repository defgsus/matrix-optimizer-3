/** @file opengl.h

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_OPENGL_H
#define MOSRC_GL_OPENGL_H

#include <iostream>

#include <Qt>

#ifdef Q_OS_UNIX
#   include <GL/glew.h>
#   include <GL/gl.h>
#endif

#include "io/error.h"


#ifdef GLEW_MX

/** needed for GLEW MX support */
GLEWContext * glewGetContext();

#endif



/** Executes the command and calls glGetError() and
    prints the error, if any. */
#define MO_CHECK_GL(command__)                  \
{                                               \
    command__;                                  \
    if (GLenum err__ = glGetError())            \
    {                                           \
        std::cerr << "opengl error "            \
            << ::MO::GL::glErrorName(err__)     \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n";        \
    }                                           \
}

/** Executes the command and throws a
    MO::GlException with @p text__ and further
    info on any errors. */
#define MO_ASSERT_GL(command__, text__)         \
{                                               \
    command__;                                  \
    if (GLenum err__ = glGetError())            \
    {                                           \
        MO_GL_ERROR(text__ << "\n(opengl error "\
            << ::MO::GL::glErrorName(err__)     \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << ")");        \
    }                                           \
}

namespace MO {
namespace GL {

    const char * glErrorName(GLenum error);

    const GLuint invalidGl = -1;

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_OPENGLFUNCTIONS_H
