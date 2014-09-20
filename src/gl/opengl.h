/** @file opengl.h

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_OPENGL_H
#define MOSRC_GL_OPENGL_H

#include <iostream>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>

#include "opengl_fwd.h"
#include "io/error.h"


#ifndef NDEBUG
/** Executes the command and calls glGetError() and
    prints the error, if any. */
#define MO_CHECK_GL(command__)                  \
{                                               \
    command__;                                  \
    ::gl::GLenum err__(::gl::glGetError());     \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        std::cerr << "opengl error "            \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n";        \
    }                                           \
}
#else
#   define MO_CHECK_GL(command__)               \
    { command__; }
#endif



#ifndef NDEBUG
/** Executes the command and calls glGetError().
    On an error, report__ (of type MO::GL::ErrorReporting)
    will define what happens. */
#define MO_CHECK_GL_COND(report__, command__)   \
{                                               \
    command__;                                  \
    ::gl::GLenum err__ = ::gl::glGetError();    \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        std::stringstream s__;                  \
        s__ << "OPENGL ERROR "                  \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__;                \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__.str()              \
                      << std::endl; else        \
        if (report__ == ::MO::GL::ER_THROW)     \
            MO_GL_ERROR(s__.str());             \
    }                                           \
}
#else
#define MO_CHECK_GL_COND(report__, command__)   \
{                                               \
    command__;                                  \
    ::gl::GLenum err__ = ::gl::glGetError();    \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        std::stringstream s__;                  \
        s__ << "OPENGL ERROR "                  \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__;                      \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__.str()              \
                      << std::endl; else        \
        if (report__ == ::MO::GL::ER_THROW)     \
            MO_GL_ERROR(s__.str());             \
    }                                           \
}
#endif

#ifndef NDEBUG
/** Executes the command, calls glGetError() and returns
    the error in ret__.
    On an error, report__ (of type MO::GL::ErrorReporting)
    will define what happens. */
#define MO_CHECK_GL_RET_COND(report__, command__, ret__) \
{                                               \
    command__;                                  \
    ret__ = ::gl::glGetError();                 \
    if (ret__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        std::stringstream s__;                  \
        s__ << "opengl error "                  \
            << ::MO::GL::errorName(ret__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__;                \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__.str()              \
                      << std::endl; else        \
        if (report__ == ::MO::GL::ER_THROW)     \
            MO_GL_ERROR(s__.str());             \
    }                                           \
}
#else
/** Executes the command, calls glGetError() and returns
    the error in ret__.
    On an error, report__ (of type MO::GL::ErrorReporting)
    will define what happens. */
#define MO_CHECK_GL_RET_COND(report__, command__, ret__) \
{                                               \
    command__;                                  \
    ret__ = ::gl::glGetError();                 \
    if (ret__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        std::stringstream s__;                  \
        s__ << "opengl error "                  \
            << ::MO::GL::errorName(ret__)       \
            << " for command "                  \
            << #command__;                      \
        if (report__ == ::MO::GL::ER_IGNORE)    \
            std::cerr << s__.str()              \
                      << std::endl; else        \
        if (report__ == ::MO::GL::ER_THROW)     \
            MO_GL_ERROR(s__.str());             \
    }                                           \
}
#endif


#ifndef NDEBUG
/** Executes the command and throws a
    MO::GlException with @p text__ and further
    info on any errors. */
#define MO_ASSERT_GL(command__, text__)         \
{                                               \
    command__;                                  \
    ::gl::GLenum err__ = ::gl::glGetError();    \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR(text__ << "\n(opengl error "\
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << ")");        \
    }                                           \
}
#else
#define MO_ASSERT_GL(command__, text__)         \
{                                               \
    command__;                                  \
    ::gl::GLenum err__ = ::gl::glGetError();    \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR(text__ << "\n(opengl error "\
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__ << ")");              \
    }                                           \
}
#endif


namespace MO {
namespace GL {

    //bool operator == (const ::gl::GLenum & lhs, const ::gl::GLenum & rhs);

    /** Used to flag invalid GLuint names */
    const gl::GLuint invalidGl = -1;

    /** Need to be called, once a context is ready */
    void moInitGl();

    /** Returns the readable name of the error */
    const char * errorName(gl::GLenum error);

    /** Returns the size in bytes of an openGL type enum (like GL_FLOAT) */
    gl::GLuint typeSize(gl::GLenum);

    /** Returns the number of channels for a given
        enum like GL_RED, GL_RGB, or GL_RGBA...
        returns 0, if type is not known! */
    gl::GLuint channelSize(gl::GLenum channel_format);

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_OPENGLFUNCTIONS_H
