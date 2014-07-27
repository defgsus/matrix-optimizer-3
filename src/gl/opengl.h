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
#endif

#include "context.h"

#include "io/error.h"


#ifdef GLEW_MX

/** needed for GLEW MX support */
GLEWContext * glewGetContext();

#endif

// TODO: remove from release version

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

// TODO: remove __FILE__ and __LINE__ from release version

/** Executes the command and calls glGetError().
    On an error, report__ (of type MO::GL::ErrorReporting)
    will define what happens. */
#define MO_CHECK_GL_COND(report__, command__)   \
{                                               \
    command__;                                  \
    if (GLenum err__ = glGetError())            \
    {                                           \
        std::stringstream s__;                  \
        s__ << "opengl error "                  \
            << ::MO::GL::glErrorName(err__)     \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n";        \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__; else              \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            MO_GL_ERROR(s__);                   \
    }                                           \
}

/** Executes the command, calls glGetError() and returns
    the error in ret__.
    On an error, report__ (of type MO::GL::ErrorReporting)
    will define what happens. */
#define MO_CHECK_GL_RET_COND(report__, command__, ret__) \
{                                               \
    command__;                                  \
    if ((ret__ = glGetError()))                 \
    {                                           \
        std::stringstream s__;                  \
        s__ << "opengl error "                  \
            << ::MO::GL::glErrorName(ret__)     \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n";        \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__; else              \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            MO_GL_ERROR(s__);                   \
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

    /** Used to flag invalid GLuint names */
    const GLuint invalidGl = -1;

    enum ErrorReporting
    {
        ER_IGNORE,
        ER_THROW
    };

    /** Need to be called, once a context is ready */
    void moInitGl();

    /** Returns the readable name of the error */
    const char * glErrorName(GLenum error);

    /** Returns the size in bytes of an openGL type enum (like GL_FLOAT) */
    GLuint typeSize(GLenum);

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_OPENGLFUNCTIONS_H
