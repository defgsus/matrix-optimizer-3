/** @file openglfunctions.h

    @brief includes QOpenGLFunctions_x_x

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MOSRC_GL_OPENGLFUNCTIONS_H
#define MOSRC_GL_OPENGLFUNCTIONS_H

#define MO_QOPENGL_FUNCTIONS_CLASS QOpenGLFunctions_3_0
#include <QOpenGLFunctions_3_0>



/** Executes the command and calls glGetError() and
    prints the error, if any. */
#define MO_CHECK_GL(funcs__, command__)         \
{                                               \
    funcs__->command__;                         \
    if (GLenum err__ = funcs__->glGetError())   \
    {                                           \
        std::cerr << "opengl error "            \
            << ::MO::GL::glErrorName(err__)     \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n";        \
    }                                           \
}

namespace MO {
namespace GL {

    const char * glErrorName(GLenum error);

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_OPENGLFUNCTIONS_H
