/** @file opengl.cpp

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include "opengl.h"
#include "compatibility.h"
#include "io/log.h"

#ifdef GLEW_MX

#include <pthread.h>

    static pthread_key_t glew_key_;
    static pthread_once_t glew_key_once_ = PTHREAD_ONCE_INIT;


    static void make_glew_key_()
    {
        (void) pthread_key_create(&glew_key_, NULL);
    }

    GLEWContext * glewGetContext()
    {
        GLEWContext * con = 0;

        // create the key
        pthread_once(&glew_key_once_, make_glew_key_);

        void *ptr;
        if ((ptr = pthread_getspecific(glew_key_)) == NULL)
        {
            MO_DEBUG_GL("creating GLEWContext object");

            // create context
            con = new GLEWContext;
            memset(con, 0, sizeof(GLEWContext));
            ptr = con;
            // store to thread
            pthread_setspecific(glew_key_, ptr);

            glewInit();
        }
        else
        {
            //MO_DEBUG_GL("reusing GLEWContext object");
            con = static_cast<GLEWContext*>(ptr);
        }

        return con;
    }

#endif // GLEW_MX



namespace MO {
namespace GL {

void moInitGl()
{
    static bool init = false;
    if (!init)
    {
        MO_DEBUG_GL("Initializing GLEW (single-threaded)");

        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            QString str;
            if (err == GLEW_ERROR_NO_GL_VERSION)
                str = "missing GL version";
            else if (err == GLEW_ERROR_GL_VERSION_10_ONLY)
                str = "Need at least OpenGL 1.1";
            else if (err == GLEW_ERROR_GLX_VERSION_11_ONLY)
                str = "Need at least GLX 1.2";
            else str = "Unknown error";

            MO_GL_ERROR("Could not initialize GLEW\n" << str);
        }

        checkCompatibility();

        init = true;
    }
}


const char * errorName(GLenum error)
{
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

GLuint typeSize(GLenum t)
{
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
        default: return 0;
    }
}


GLuint channelSize(GLenum channel_format)
{
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
            return 0;
    }
}


} // namespace GL
} // namespace MO
