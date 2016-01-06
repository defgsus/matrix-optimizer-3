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

#ifdef NDEBUG

/** Executes the command and calls glGetError() and
    throws GlException on error. */
#define MO_CHECK_GL_THROW(command__)            \
{                                               \
    command__;                                  \
    ::gl::GLenum err__(::gl::glGetError());     \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR("opengl error "             \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__);                     \
    }                                           \
}

/** Executes the command and calls glGetError() and
    throws GlException on error, adds text__ (stream arguments) */
#define MO_CHECK_GL_THROW_TEXT(command__, text__)\
{                                               \
    command__;                                  \
    ::gl::GLenum err__(::gl::glGetError());     \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR("opengl error "             \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__ << "\n"               \
            << text__);                         \
    }                                           \
}

#else

/** Executes the command and calls glGetError() and
    throws GlException on error. */
#define MO_CHECK_GL_THROW(command__)            \
{                                               \
    command__;                                  \
    ::gl::GLenum err__(::gl::glGetError());     \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR("opengl error "             \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__);               \
    }                                           \
}

/** Executes the command and calls glGetError() and
    throws GlException on error, adds text__ (stream arguments) */
#define MO_CHECK_GL_THROW_TEXT(command__, text__)\
{                                               \
    command__;                                  \
    ::gl::GLenum err__(::gl::glGetError());     \
    if (err__ != ::gl::GL_NO_ERROR)             \
    {                                           \
        MO_GL_ERROR("opengl error "             \
            << ::MO::GL::errorName(err__)       \
            << " for command "                  \
            << #command__                       \
            << " in " << __FILE__               \
            << ": " << __LINE__ << "\n"         \
            << text__);                         \
    }                                           \
}

#endif



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

    /** Returns e.g. GL_RGBA for GL_RGBAF32 and all it's variants */
    gl::GLenum inputFormat(gl::GLenum);

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_OPENGLFUNCTIONS_H
