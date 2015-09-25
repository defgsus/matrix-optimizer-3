/** @file texturerenderer.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 16.05.2015</p>
*/

#ifndef MOSRC_GL_TEXTURERENDERER_H
#define MOSRC_GL_TEXTURERENDERER_H

#include "types/int.h"
#include "gl/opengl_fwd.h"

namespace MO {
namespace GL {

/** Convenience wrapper to rescale textures */
class TextureRenderer
{
public:
    TextureRenderer(uint w = 256, uint h = 256);
    ~TextureRenderer();

    /** Requests a new size.
        Which is applied on createGl() or render() */
    void setSize(uint w, uint h);

    bool isGlInitialized() const;

    // --------- opengl -----------

    /** Renders the texture onto the fbo.
        The texture will be bound only if @p bindTexture is true.
        The internal fbo will be created or resized as needed.
        Opengl context must match the context of the texture.
        @throws GlException */
    void render(const Texture * , bool bindTexture = false);

    /** Explicitely create/resize the fbo.
        @throws GlException */
    void createGl();
    /** Wipe out all opengl resources */
    void releaseGl();

    /** Returns the texture in the internal fbo */
    const Texture * texture() const;

private:

    FrameBufferObject * fbo_;
    ScreenQuad * quad_, * fquad_;
    uint w_, h_;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_TEXTURERENDERER_H
