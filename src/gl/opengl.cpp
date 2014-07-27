/** @file opengl.cpp

    @brief Basic opengl include (uses GLEWMX)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include "opengl.h"
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

const char * glErrorName(GLenum error)
{
    switch (error)
    {
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        //case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default: return "UNKNOWN_ERROR";
    }
}

} // namespace GL
} // namespace MO
