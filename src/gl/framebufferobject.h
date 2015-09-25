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

    enum Attachment
    {
        A_DEPTH = 1
    };

    explicit FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            bool cubemap = false, bool multiSample = false);

    explicit FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            bool cubemap = false, bool multiSample = false);

    explicit FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            int attachmentMask,
            bool cubemap = false, bool multiSample = false);

    explicit FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            int attachmentMask,
            bool cubemap = false, bool multiSample = false);

    explicit FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            int attachmentMask,
            bool cubemap = false, bool multiSample = false);

    ~FrameBufferObject();

    /** Adds @p num additional color render targets.
        Must be called before create() */
    void addColorTextures(uint num);

    // ------------- getter ----------------------

    bool isCreated() const { return fbo_ != invalidGl; }

    /** If true, the color-texture is a texture cube */
    bool isCubemap() const { return isCubemap_; }

    uint width() const;
    uint height() const;
    /** Depth of texture3d, 1 if texture2d. */
    uint depth() const;
    uint numColorTextures() const { return colorTex_.size(); }

    /** Returns the associated color texture */
    const Texture * colorTexture(uint index = 0) const { return colorTex_[index]; }
    /** Returns the associated color texture, or NULL */
    const Texture * depthTexture() const { return depthTex_; }

    /** Returns the associated color texture.
        The texture is detached from the fbo and will not be released or used further.
        You need to call create() to make the fbo useable again. */
    Texture * takeColorTexture(uint index = 0);
    Texture * takeDepthTexture();

    float aspect() { return float(width()) / std::max(1u, height()); }

    // ------------- setter ----------------------

    const QString& name() const { return name_; }
    void setName(const QString& name);
    /** Calls Texture::setChanged for all attached textures */
    void setChanged();

    // ------------ opengl interface -------------

    /** @throws GlException */
    void bind();
    /** @throws GlException */
    void unbind();

    /** Creates the framebuffer, renderbuffer and color-texture
        as previously defined.
        @throws GlException */
    void create();

    /** Releases the opengl resources. */
    void release();

    /** Replaces the current color texture with @p t and returns the unused texture.
        @note Error handling is bad here, if something goes wrong the fbo is probably unuseable.
        @note Settings and formats must match and fbo must be bound! */
    Texture * swapColorTexture(Texture * t, uint index = 0);

    /** Attach one of the cubemap textures to the colorbuffer.
        @p target is of type GL_TEXTURE_CUBE_MAP_[POSITIVE|NEGATIVE]_[X|Y|Z].
        Prior, create() must have been successfully called and
        the framebuffer object must be in cubemap mode.
        @throws GlException */
    void attachCubeTexture(gl::GLenum target, uint index = 0);

    /** Downloads the color texture from the device.
        NOT available for cubemap textures.
        @throws GlException*/
    void downloadColorTexture(void * ptr, uint index = 0);
    void downloadDepthTexture(void * ptr);

    /** Convenience function sets opengl viewport to (0,0, width, height).
        @throws GlException */
    void setViewport() const;

private:

    /** Releases the fbo, rbo and texture */
    void release_();

    std::vector<Texture*> colorTex_;
    Texture * depthTex_;

    gl::GLuint fbo_, rbo_;

    int attachments_;
    bool isCubemap_, isMultiSample_;

    QString name_;
};



} // namespace GL
} // namespace MO


#endif // MOSRC_GL_FRAMEBUFFEROBJECT_H
