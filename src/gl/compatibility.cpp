/** @file compatibility.cpp

    @brief OpenGL function wrappers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include <sstream>

#include "compatibility.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GL {

namespace
{
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


bool checkCompatibility()
{
    Properties& p = Properties::staticInstance();
    p.getProperties();

    MO_DEBUG("OPENGL PROPERTIES:\n" << p.toString());

    return p.versionMajor >= 3;
}



Properties& Properties::staticInstance()
{
    static Properties * p_ = 0;
    if (!p_)
        p_ = new Properties;
    return *p_;
}

void Properties::getProperties()
{
    // clear any previous errors
    glGetError();

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

    stringVendor = (const char *)glGetString(GL_VENDOR),
    stringVersion = (const char *)glGetString(GL_VERSION),
    stringRenderer = (const char *)glGetString(GL_RENDERER);

    isMesa = stringRenderer.contains("Mesa", Qt::CaseInsensitive);

    // sorry mesa guys, but it looks terrible...
    canLineSmooth = !isMesa;

    MO_CHECK_GL( glGetIntegerv(GL_MAJOR_VERSION, &versionMajor) );
    MO_CHECK_GL( glGetIntegerv(GL_MINOR_VERSION, &versionMinor) );

    // primitive sizes
    MO_CHECK_GL( glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidth) );
    MO_CHECK_GL( glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidthSmooth) );
    MO_CHECK_GL( glGetIntegerv(GL_POINT_SIZE_RANGE, pointSize) );

    // texture
    MO_CHECK_GL( glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize) );
    MO_CHECK_GL( glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits) );

    // uniforms
    MO_CHECK_GL( glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &maxUniformLocations) );
    MO_CHECK_GL( glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxUniformBlocksVertex) );
    MO_CHECK_GL( glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxUniformBlocksFragment) );
    MO_CHECK_GL( glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &maxUniformBlocksGeometry) );
    MO_CHECK_GL( glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockBytes) );

    // ------- empirical tests ---------

    glGetError();

    // some drivers complain, even if they report a possible range
    glLineWidth(lineWidth[1]);
    canLineWidth = glGetError() == GL_NO_ERROR;
}

QString Properties::toString() const
{
    std::stringstream s;

    s   << "vendor:                    " << stringVendor << "\n"
        << "version:                   " << stringVersion << "\n"
        << "renderer:                  " << stringRenderer << "\n"
        << "line width:                " << lineWidth[0] << "-" << lineWidth[1] << "\n"
        << "line width smooth:         " << lineWidthSmooth[0] << "-" << lineWidthSmooth[1] << "\n"
        << "point size:                " << pointSize[0] << "-" << pointSize[1] << "\n"

        << "texture size:              " << maxTextureSize << "\n"
        << "texture units:             " << maxTextureUnits << "\n"

        << "uniform locations:         " << maxUniformLocations << "\n"
        << "uniform blocks per shader: vert: " << maxUniformBlocksFragment
                                    << ", frag: " << maxUniformBlocksFragment
                                    << ", geom: " << maxUniformBlocksGeometry << "\n"
        << "uniform block size:        " << maxUniformBlockBytes << " bytes\n"
    ;

    return QString::fromStdString(s.str());
}



// --------------------------- setter -----------------------------------


void Properties::setLineSmooth(bool enable)
{
    if (!canLineSmooth)
        return;

    if (enable)
        MO_CHECK_GL( glEnable(GL_LINE_SMOOTH) )
    else
        MO_CHECK_GL( glDisable(GL_LINE_SMOOTH) );
}

void Properties::setLineWidth(GLfloat width)
{
    if (!canLineWidth)
        return;

    GLboolean isSmooth;
    MO_CHECK_GL( glGetBooleanv(GL_LINE_SMOOTH, &isSmooth) );

    if (isSmooth == GL_TRUE)
        width = std::max(GLfloat(lineWidthSmooth[0]), std::min(GLfloat(lineWidthSmooth[1]), width ));
    else
        width = std::max(GLfloat(lineWidth[0]), std::min(GLfloat(lineWidth[1]), width ));

#if 0
    MO_CHECK_GL( glLineWidth(width) );
#else
    MO_DEBUG("width " << width);
    glLineWidth(width);
    if (glGetError() != GL_NO_ERROR)
        MO_DEBUG("glLineWidth(" << width << ") failed, smooth = " << int(isSmooth) << ", range = "
                 << lineWidth[0] << "-" << lineWidth[1] << ", srange = "
                 << lineWidthSmooth[0] << "-" << lineWidthSmooth[1]);
#endif
}

void Properties::setPointSize(GLfloat s)
{
    s = std::max(GLfloat(pointSize[0]), std::min(GLfloat(pointSize[1]), s ));

    MO_CHECK_GL( gl::glPointSize(s) );
}

} // namespace GL
} // namespace MO
