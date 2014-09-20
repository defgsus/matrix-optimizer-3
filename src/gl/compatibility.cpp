/** @file compatibility.cpp

    @brief OpenGL function wrappers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include "compatibility.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GL {

namespace
{
    static bool
        enableLineSmooth_ = false,
        enableLineWidth_ = false;

    void dumpExtensions()
    {
        GLint num=0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num);

        MO_PRINT(num << " opengl extensions:");
        for (int i=0; i<num; ++i)
        {
            QString ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
            MO_PRINT(ext);
        }
    }
}

void checkCompatibility()
{
    const QString
            vendor = (const char *)glGetString(GL_VENDOR),
            version = (const char *)glGetString(GL_VERSION),
            renderer = (const char *)glGetString(GL_RENDERER);

    MO_DEBUG(     "vendor   " << vendor
                << "\nversion  " << version
                << "\nrenderer " << renderer);

    //dumpExtensions();
/*
#define MO__REQUIRE(str__)                                  \
    if (!glewIsSupported(str__))                            \
    {                                                       \
        MO_PRINT("Sorry, but " str__ " is required.");      \
        exit(-1);                                           \
    }

    MO__REQUIRE("GL_VERSION_3_0");
    //MO__REQUIRE("GL_ARB_framebuffer_object");
    //MO__REQUIRE("GL_ARB_vertex_array_object");

#undef MO__REQUIRE
*/

    bool isMesa = renderer.contains("Mesa", Qt::CaseInsensitive);

    // sorry mesa guys, but...
    enableLineSmooth_ = !isMesa;

    // test line width
    glLineWidth(2);
    enableLineWidth_ = (glGetError() == 0);
    glLineWidth(1);
}

void setLineSmooth(bool enable)
{
    if (!enableLineSmooth_)
        return;

    if (enable)
        MO_CHECK_GL( glEnable(GL_LINE_SMOOTH) )
    else
        MO_CHECK_GL( glDisable(GL_LINE_SMOOTH) );
}

void setLineWidth(gl::GLfloat width)
{
    if (!enableLineWidth_)
        return;

    MO_CHECK_GL( glLineWidth(width) );
}


} // namespace GL
} // namespace MO
