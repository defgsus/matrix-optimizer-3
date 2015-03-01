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
#include "io/streamoperators_glbinding.h"

using namespace gl;

namespace MO {
namespace GL {

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type, bool cubemap,
            ErrorReporting report)
    : FrameBufferObject(width, height, format, type, 0, cubemap, report)
{
}


FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            int attachmentMask, bool cubemap,
            ErrorReporting report)
    : rep_          (report),
      colorTex_     (0),
      depthTex_     (0),
      fbo_          (invalidGl),
      rbo_          (invalidGl),
      attachments_  (attachmentMask),
      cubemap_      (cubemap)
{
    MO_DEBUG_GL("FrameBufferObject::FrameBufferObject("
                << width << ", " << height << ", " << format << ", " << type <<
                ", " << cubemap << ")");

    if (!cubemap_)
        colorTex_ = new Texture(width, height, format, format, type, 0, report);
    else
    {
        colorTex_ = new Texture(width, height, format, format, type, 0, 0, 0, 0, 0, 0, report);
    }

    if (attachments_ & A_DEPTH)
    {
        if (!cubemap_)
            depthTex_ = new Texture(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0, report);
        else
        {
            depthTex_ = new Texture(width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0, 0, 0, 0, 0, report);
        }
    }
}

FrameBufferObject::~FrameBufferObject()
{
    MO_DEBUG_GL("FrameBufferObject::~FrameBufferObject()");

    if (isCreated())
        MO_GL_WARNING("destructor of unreleased FrameBufferObject");

    delete depthTex_;
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
    int pixelsize = 1; //devicePixelRatio(); // Retina support
    MO_DEBUG_GL("FrameBufferObject::setViewport()")
    MO_CHECK_GL_COND(rep_, glViewport(0,0, width()*pixelsize, height()*pixelsize) );
}


bool FrameBufferObject::bind()
{
    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glBindFramebuffer(GL_FRAMEBUFFER, fbo_), err );
    if (err != GL_NO_ERROR) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindRenderbuffer(GL_RENDERBUFFER, rbo_), err );
    return err == GL_NO_ERROR;
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

    if (depthTex_ && depthTex_->isCreated())
        depthTex_->release();
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
        MO_GL_ERROR_COND(rep_, "No size given for frame buffer object creation");
        return false;
    }

    if (!colorTex_->isCreated())
    {
        if (!colorTex_->create())
        {
            MO_GL_ERROR_COND(rep_, "could not create colorbuffer for framebuffer");
            return false;
        }
    }

    if (depthTex_ && !depthTex_->isCreated())
    {
        if (!depthTex_->create())
        {
            MO_GL_ERROR_COND(rep_, "could not create depthbuffer for framebuffer");
            return false;
        }
    }

    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glGenFramebuffers(1, &fbo_), err );
    if (err != GL_NO_ERROR) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindFramebuffer(GL_FRAMEBUFFER, fbo_), err );
    if (err != GL_NO_ERROR) return false;

    // attach color texture
    GLenum target = cubemap_? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : GL_TEXTURE_2D;
    MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, colorTex_->handle(), 0), err );
    if (err != GL_NO_ERROR) return false;

    // for depth testing
    MO_CHECK_GL_RET_COND(rep_, glGenRenderbuffers(1, &rbo_), err );
    if (err != GL_NO_ERROR) return false;
    MO_CHECK_GL_RET_COND(rep_, glBindRenderbuffer(GL_RENDERBUFFER, rbo_), err );
    if (err != GL_NO_ERROR) return false;
    if (attachments_ & A_DEPTH)
    {
        // attach depth texture
        MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTex_->handle(), 0), err );
        if (err != GL_NO_ERROR) return false;
    }
    else
    {
        // attach internal storage for depth
        MO_CHECK_GL_RET_COND(rep_, glRenderbufferStorage(
                GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, colorTex_->width(), colorTex_->height() ), err );
        if (err != GL_NO_ERROR) return false;
        MO_CHECK_GL_RET_COND(rep_, glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_), err );
        if (err != GL_NO_ERROR) return false;
    }

    MO_DEBUG_GL("FrameBufferObject::create()")
    MO_CHECK_GL_COND(rep_, glViewport(0, 0, width(), height()) );
    MO_CHECK_GL_COND(rep_, glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    return true;
}

void FrameBufferObject::release()
{
    release_();
}

bool FrameBufferObject::attachCubeTexture(gl::GLenum target)
{
    if (!cubemap_)
    {
        MO_GL_ERROR_COND(rep_, "FrameBufferObject::attachCubeTexture() for non-cube texture");
        return false;
    }

    if (!colorTex_ || !colorTex_->isAllocated())
    {
        MO_GL_ERROR_COND(rep_, "FrameBufferObject::attachCubeTexture() on uninitialized cube-map");
        return false;
    }

    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, colorTex_->handle(), 0), err );
    if (err != GL_NO_ERROR)
        return false;
    if (attachments_ & A_DEPTH)
    {
        MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTex_->handle(), 0), err );
    }
    return err == GL_NO_ERROR;

}

bool FrameBufferObject::downloadColorTexture(void *ptr)
{
    return colorTex_->download(ptr);
}

Texture * FrameBufferObject::takeColorTexture()
{
    Texture * t = colorTex_;

    // create new one
    if (!cubemap_)
        colorTex_ = new Texture(width(), height(), colorTex_->format(), colorTex_->format(), colorTex_->type(), 0, rep_);
    else
    {
        colorTex_ = new Texture(width(), height(), colorTex_->format(), colorTex_->format(), colorTex_->type(),
                                0, 0, 0, 0, 0, 0, rep_);
    }

    return t;
}

bool FrameBufferObject::downloadDepthTexture(void *ptr)
{
    return depthTex_ ? depthTex_->download(ptr) : 0;
}

Texture * FrameBufferObject::takeDepthTexture()
{
    if (!depthTex_)
        return 0;

    Texture * t = depthTex_;

    // create new one
    if (!cubemap_)
        depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(), colorTex_->type(), 0, rep_);
    else
    {
        depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(), colorTex_->type(),
                                0, 0, 0, 0, 0, 0, rep_);
    }

    return t;
}

} // namespace GL
} // namespace MO
