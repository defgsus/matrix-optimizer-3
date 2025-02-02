/** @file opengl.cpp

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <map>
#include <mutex>
#include <thread>

#include "opengl.h"
#include "compatibility.h"
#include "io/log_gl.h"

namespace MO {
namespace GL {

namespace
{
    static std::mutex initMutex_;
    static std::map<std::thread::id, bool> isInitMap_;
}

void moInitGl()
{
    std::lock_guard<std::mutex> lock(initMutex_);

    auto id = std::this_thread::get_id();
    if (isInitMap_.find(id) == isInitMap_.end())
    {
        MO_DEBUG_GL("Initializing glBinding");
        glbinding::Binding::initialize();
        isInitMap_.insert(std::make_pair(id, true));
    }

    static bool init = false;
    if (!init)
    {
        checkCompatibility();

        init = true;
    }
}

void moCloseGl()
{
    MO_DEBUG_GL("moCloseGl()");

    std::lock_guard<std::mutex> lock(initMutex_);

    auto id = std::this_thread::get_id();
    isInitMap_.erase(id);
    glbinding::Binding::releaseCurrentContext();
}


const char * errorName(gl::GLenum error)
{
    using namespace gl;

    switch (error)
    {
        case gl::GLenum(0): return "NO_ERROR";
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
        case GL_R:
        case GL_R8:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_LUMINANCE:
        case GL_ALPHA:
            return 1;

        case GL_RG:
        case GL_RG_INTEGER:
        case GL_RG8:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
        case GL_LUMINANCE_ALPHA:
            return 2;

        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB32F:
            return 3;

        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA32F:	return 4;

        default:
            return 1;
    }
}

gl::GLenum inputFormat(gl::GLenum fmt)
{
    using namespace gl;

    switch (fmt)
    {
        case GL_R:
        case GL_RED:
        case GL_R16:
        case GL_R16F:
        case GL_R32F:
            return GL_RED;
        break;

        case GL_RG:
        case GL_RG16:
        case GL_RG16F:
        case GL_RG32F:
            return GL_RG;
        break;

        case GL_RGB:
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB32F:
            return GL_RGB;
        break;

        case GL_RGBA:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA32F:
        default:
            return GL_RGBA;
        break;
    }
}

} // namespace GL
} // namespace MO
