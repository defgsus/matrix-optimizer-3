/** @file compatibility.h

    @brief OpenGL function wrappers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#ifndef MOSRC_GL_COMPATIBILITY_H
#define MOSRC_GL_COMPATIBILITY_H

#include "opengl.h"

namespace MO {
namespace GL {

/** Checks for compatibility of the current driver.
    This will create and initialize Properties::staticInstance() */
bool checkCompatibility();


/** Container for OpenGL property querries */
class Properties
{
public:

    /** Returns a default property instance initialized during first context creation */
    static Properties& staticInstance();

    /** Initializes the members to the properties of the current context */
    void getProperties();

    /** Returns a descriptive string with all properties */
    QString toString() const;

    // --------------- properties --------------------------

    QString stringVendor,
            stringVersion,
            stringRenderer;

    /** Is smoothed lines possible */
    bool canLineSmooth,
    /** Is this a mesa driver? */
         isMesa;

    gl::GLint
            versionMajor,
            versionMinor,
    /** The minimum and maximum (aliased) line width */
            lineWidth[2],
    /** The minimum and maximum (anti-aliased) line width */
            lineWidthSmooth[2],
    /** The minimum and maximum point size (for GL_POINTS) */
            pointSize[2],
    /** Maximum width and height of a texture */
            maxTextureSize,
    /** Maximum bindable texture units */
            maxTextureUnits,
    /** The number of explicitly assignable uniform locations */
            maxUniformLocations,
    /** Maximum number of uniform buffer objects per shader*/
            maxUniformBlocksVertex,
    /** Maximum number of uniform buffer objects per shader*/
            maxUniformBlocksFragment,
    /** Maximum number of uniform buffer objects per shader*/
            maxUniformBlocksGeometry,
    /** Maximum number of bytes in uniform buffer objects */
            maxUniformBlockBytes;

    // --------------- opengl state setter ----------------

    /** Enables or disables GL_LINE_SMOOTH */
    void setLineSmooth(bool enable);

    /** Sets the line-width */
    void setLineWidth(gl::GLfloat width);

    void setPointSize(gl::GLfloat);
};

} // namespace GL
} // namespace MO

#endif // COMPATIBILITY_H
