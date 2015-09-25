/** @file opengl.cpp

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include "opengl.h"
#include "compatibility.h"
#include "io/log.h"

namespace MO {
namespace GL {

void moInitGl()
{
    static bool init = false;
    if (!init)
    {
        MO_DEBUG_GL("Initializing glBinding");

        glbinding::Binding::initialize();

        checkCompatibility();

        init = true;
    }
}


const char * errorName(gl::GLenum error)
{
    using namespace gl;

    switch (error)
    {
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default: return "UNKNOWN_ERROR";
    }
}

gl::GLuint typeSize(gl::GLenum t)
{
    using namespace gl;

    switch (t)
    {
        case GL_INT:
        case GL_UNSIGNED_INT: return sizeof(GLint);
        case GL_SHORT:
        case GL_UNSIGNED_SHORT: return sizeof(GLshort);
        case GL_BYTE:
        case GL_UNSIGNED_BYTE: return sizeof(GLbyte);
        case GL_FLOAT: return sizeof(GLfloat);
        case GL_DOUBLE: return sizeof(GLdouble);
        case GL_2_BYTES: return 2*sizeof(GLbyte);
        case GL_3_BYTES: return 3*sizeof(GLbyte);
        case GL_4_BYTES: return 4*sizeof(GLbyte);
        default: return 1;
    }
}


gl::GLuint channelSize(gl::GLenum channel_format)
{
    using namespace gl;

    switch (channel_format)
    {
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_LUMINANCE:
        case GL_ALPHA: 		return 1;

        case GL_LUMINANCE_ALPHA: return 2;

        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:		return 3;

        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA32F:	return 4;

        default:
            return 1;
    }
}


} // namespace GL
} // namespace MO
