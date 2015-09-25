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



TextureRenderer::TextureRenderer(uint w, uint h)
    : fbo_      (0),
      quad_     (0),
      fquad_    (0),
      w_        (w),
      h_        (h)
{
}

TextureRenderer::~TextureRenderer()
{
    delete fquad_;
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

void TextureRenderer::createGl()
{
    // create instance
    if (!fbo_)
    {
        fbo_ = new FrameBufferObject(w_, h_, gl::GL_RGBA, gl::GL_FLOAT, false);
    }

    // update size
    if (fbo_->width() != w_ || fbo_->height() != h_)
        fbo_->release();

    // create opengl object
    if (!fbo_->isCreated())
    {
        fbo_->create();
    }

    // create quad instance
    if (!quad_)
        quad_ = new ScreenQuad("texrender");

    // create opengl object
    if (!quad_->isCreated())
    {
        try
        {
            quad_->create("#define MO_ANTIALIAS 4");
        }
        catch (Exception& e)
        {
            e << "\nin creating quad for TextureRenderer";
            releaseGl();
            throw;
        }
    }

    // create quad instance
    if (!fquad_)
        fquad_ = new ScreenQuad("ftexrender");

    // create opengl object
    if (!fquad_->isCreated())
    {
        try
        {
            fquad_->create("#define MO_ANTIALIAS 4\n#define MO_FULLDOME_CUBE");
        }
        catch (Exception& e)
        {
            e << "\nin creating quad for TextureRenderer";
            releaseGl();
            throw;
        }
    }
}

void TextureRenderer::releaseGl()
{
    if (fquad_ && fquad_->isCreated())
        fquad_->release();
    delete fquad_;
    fquad_ = 0;

    if (quad_ && quad_->isCreated())
        quad_->release();
    delete quad_;
    quad_ = 0;

    if (fbo_ && fbo_->isCreated())
        fbo_->release();
    delete fbo_;
    fbo_ = 0;
}

void TextureRenderer::render(const Texture * tex, bool bindTexture)
{
    using namespace gl;

    // update fbo
    if (!fbo_ || !fbo_->isCreated() || fbo_->width() != w_ || fbo_->height() != h_)
    {
        createGl();
    }

    fbo_->bind();

    // prepare fbo
    fbo_->setViewport();
    MO_CHECK_GL_THROW( glClearColor(0, 0, 0, 1) );
    MO_CHECK_GL_THROW( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    MO_CHECK_GL_THROW( glEnable(GL_BLEND) );
    MO_CHECK_GL_THROW( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

    // bind texture
    if (bindTexture)
    {
        MO_CHECK_GL_THROW( glActiveTexture(GL_TEXTURE0) );
        tex->bind();
    }

    // set interpolation mode
    tex->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_LINEAR));
    //fbo_->colorTexture()->setTexParameter(GL_TEXTURE_MAG_FILTER, GLint(GL_NEAREST));

    // set edge-clamp
    tex->setTexParameter(GL_TEXTURE_WRAP_S, GLint(GL_CLAMP_TO_EDGE));
    tex->setTexParameter(GL_TEXTURE_WRAP_T, GLint(GL_CLAMP_TO_EDGE));

    // render quad
    if (tex->isCube())
        fquad_->draw(w_, h_);
    else
        quad_->draw(w_, h_);

    fbo_->unbind();
}

} // namespace GL
} // namespace MO
