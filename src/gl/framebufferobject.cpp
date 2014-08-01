/** @file framebufferobject.cpp

    @brief Wrapper around frame buffer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#include "framebufferobject.h"
#include "texture.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {



FrameBufferObject::FrameBufferObject(
            GLsizei width, GLsizei height,
            GLenum format, GLenum type,
            ErrorReporting report)
    : rep_          (report),
      colorTex_     (new Texture(width, height, format, format, type, 0, report)),
      fbo_          (invalidGl),
      rbo_          (invalidGl)
{
    MO_DEBUG_GL("FrameBufferObject::FrameBufferObject("
                << width << ", " << height << ", " << format << ", " << type << ")");
}

FrameBufferObject::~FrameBufferObject()
{
    MO_DEBUG_GL("FrameBufferObject::~FrameBufferObject()");

    if (isCreated())
        MO_GL_WARNING("destructor of unreleased FrameBufferObject");

    delete colorTex_;
}


uint FrameBufferObject::width() const
{
    return colorTex_->width();
}

uint FrameBufferObject::height() const
{
    return colorTex_->height();
}

void FrameBufferObject::setViewport() const
{
    MO_CHECK_GL_COND(rep_, glViewport(0,0, width(), height()) );
}


bool FrameBufferObject::bind()
{
    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glBindFramebuffer(GL_FRAMEBUFFER, fbo_), err );
    if (err) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindRenderbuffer(GL_RENDERBUFFER, rbo_), err );
    return !(err);
}

void FrameBufferObject::unbind()
{
    MO_CHECK_GL_COND(rep_, glBindFramebuffer(GL_FRAMEBUFFER, 0) );
    MO_CHECK_GL_COND(rep_, glBindRenderbuffer(GL_RENDERBUFFER, 0) );
}

void FrameBufferObject::release_()
{
    MO_DEBUG_GL("FrameBufferObject::release_()");

    MO_CHECK_GL_COND(rep_, glDeleteRenderbuffers(1, &rbo_) );
    MO_CHECK_GL_COND(rep_, glDeleteFramebuffers(1, &fbo_) );
    fbo_ = invalidGl;

    if (colorTex_->isCreated())
        colorTex_->release();
}

bool FrameBufferObject::create()
{
    MO_DEBUG_GL("FrameBufferObject::create()");

    // release previous
    if (fbo_ != invalidGl)
        release_();

    // check for size setting
    if (colorTex_->width() == 0 || colorTex_->height() == 0)
    {
        if (rep_ == ER_THROW)
            MO_GL_ERROR("No size given for frame buffer object creation");
        return false;
    }

    if (!colorTex_->isCreated())
    {
        colorTex_->create();
    }

    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glGenFramebuffers(1, &fbo_), err );
    if (err) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindFramebuffer(GL_FRAMEBUFFER, fbo_), err );
    if (err) return false;

    // attach color texture
    MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex_->handle(), 0), err );
    if (err) return false;

    // for depth testing
    MO_CHECK_GL_RET_COND(rep_, glGenRenderbuffers(1, &rbo_), err );
    if (err) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindRenderbuffer(GL_RENDERBUFFER, rbo_), err );
    if (err) return false;
    MO_CHECK_GL_RET_COND(rep_, glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, colorTex_->width(), colorTex_->height() ), err );
    if (err) return false;
    MO_CHECK_GL_RET_COND(rep_, glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_), err );
    if (err) return false;

    MO_CHECK_GL_COND(rep_, glViewport(0, 0, width(), height()) );
    MO_CHECK_GL_COND(rep_, glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    return true;
}

bool FrameBufferObject::downloadColorTexture(void *ptr)
{
    return colorTex_->download(ptr);
}


} // namespace GL
} // namespace MO
