/** @file framebufferobject.h

    @brief Wrapper around frame buffer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#ifndef MOSRC_GL_FRAMEBUFFEROBJECT_H
#define MOSRC_GL_FRAMEBUFFEROBJECT_H

#include "gl/opengl.h"

namespace MO {
namespace GL {

class Texture;

/** Wrapper for a frame buffer object with attached render buffer */
class FrameBufferObject
{
public:

    explicit FrameBufferObject(
            GLsizei width, GLsizei height,
            GLenum format, GLenum type,
            ErrorReporting reporting = ER_IGNORE);

    ~FrameBufferObject();


    // ------------- getter ----------------------

    bool isCreated() const { return fbo_ != invalidGl; }

    /** Returns the associated color texture */
    const Texture * colorTexture() const { return colorTex_; }

    uint width() const;
    uint height() const;

    // ------------- setter ----------------------

    // ------------ opengl interface -------------

    bool bind();
    void unbind();

    /** Creates the framebuffer, renderbuffer and color-texture
        as previously defined. */
    bool create();

    /** Downloads the color texture from the device. */
    bool downloadColorTexture(void * ptr);

    /** Convenience function sets opengl viewport to (0,0, width, height) */
    void setViewport() const;

private:

    /** Releases the fbo, rbo and texture */
    void release_();

    ErrorReporting rep_;

    Texture * colorTex_;

    GLuint fbo_, rbo_;
};



} // namespace GL
} // namespace MO


#endif // MOSRC_GL_FRAMEBUFFEROBJECT_H
