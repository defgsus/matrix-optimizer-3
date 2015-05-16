/** @file texturerenderer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 16.05.2015</p>
*/

#include "texturerenderer.h"
#include "opengl.h"
#include "framebufferobject.h"
#include "screenquad.h"
#include "texture.h"

namespace MO {
namespace GL {



TextureRenderer::TextureRenderer(uint w, uint h, ErrorReporting rep)
    : fbo_      (0),
      quad_     (0),
      rep_      (rep),
      w_        (w),
      h_        (h)
{
}

TextureRenderer::~TextureRenderer()
{
    delete quad_;
    delete fbo_;
}

bool TextureRenderer::isGlInitialized() const
{
    return quad_ && quad_->isCreated();
}

void TextureRenderer::setSize(uint w, uint h)
{
    w_ = w;
    h_ = h;
}

const Texture * TextureRenderer::texture() const
{
    return fbo_ ? fbo_->colorTexture() : 0;
}

bool TextureRenderer::createGl()
{
    // create instance
    if (!fbo_)
    {
        fbo_ = new FrameBufferObject(w_, h_, gl::GL_RGBA, gl::GL_FLOAT, false, rep_);
    }

    // update size
    if (fbo_->width() != w_ || fbo_->height() != h_)
        fbo_->release();

    // create opengl object
    if (!fbo_->isCreated())
    {
        if (!fbo_->create())
            return false;
    }

    // create quad instance
    if (!quad_)
    {
        quad_ = new ScreenQuad("texrender", rep_);
    }

    // create opengl object
    if (!quad_->isCreated())
    {
        if (!quad_->create())
        {
            releaseGl();
            return false;
        }
    }

    return true;
}

void TextureRenderer::releaseGl()
{
    if (quad_ && quad_->isCreated())
        quad_->release();
    delete quad_;
    quad_ = 0;

    if (fbo_ && fbo_->isCreated())
        fbo_->release();
    delete fbo_;
    fbo_ = 0;
}

bool TextureRenderer::render(const Texture * tex, bool bindTexture)
{
    using namespace gl;

    GLenum err;

    // update fbo
    if (!fbo_ || !fbo_->isCreated() || fbo_->width() != w_ || fbo_->height() != h_)
    {
        if (!createGl())
            return false;
        if (!fbo_)
            return false;
    }

    if (!fbo_->bind())
        return false;

    // prepare fbo
    fbo_->setViewport();
    MO_CHECK_GL( glClearColor(1, 0, 1, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    // bind texture
    if (bindTexture)
    {
        MO_CHECK_GL_RET_COND(rep_, glActiveTexture(GL_TEXTURE0) , err);
        if (err != GL_NO_ERROR)
            return false;
        if (!tex->bind())
            return false;
    }

    // set interpolation mode
    tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    //fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    // render quad
    if (!quad_->draw(w_, h_))
        return false;


    fbo_->unbind();
    return true;
}

} // namespace GL
} // namespace MO
