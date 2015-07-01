/** @file framebufferobject.cpp

    @brief Wrapper around frame buffer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#include <atomic>

#include <QString>

#include "framebufferobject.h"
#include "texture.h"
#include "io/error.h"
#include "io/log.h"
#include "io/streamoperators_glbinding.h"

using namespace gl;

namespace MO {
namespace GL {

namespace { static std::atomic_uint_fast64_t fbo_count_(0); }

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type, bool cubemap,
            ErrorReporting report)
    : FrameBufferObject(width, height, 1, format, format, type, 0, cubemap, report)
{

}

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum in_format, gl::GLenum type,
            bool cubemap,
            ErrorReporting report)
    : FrameBufferObject(width, height, 1, format, in_format, type, 0, cubemap, report)
{

}

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            int attachmentMask, bool cubemap,
            ErrorReporting report)
    : FrameBufferObject(width, height, 1, format, format, type, attachmentMask, cubemap, report)
{

}

FrameBufferObject::FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            int attachmentMask, bool cubemap,
            ErrorReporting report)
    : FrameBufferObject(width, height, 1, format, input_format, type, attachmentMask, cubemap, report)
{

}

FrameBufferObject::FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
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
                << width << "x" << height << "x" << depth
                << ", " << format << ", " << type
                << ", " << cubemap << ")");

    colorTex_.resize(1);
    if (!cubemap_)
    {
        if (depth < 2)
            colorTex_[0] = new Texture(width, height, format, input_format, type, 0, report);
        else
            colorTex_[0] = new Texture(width, height, depth, format, input_format, type, 0, report);
    }
    else
    {
        colorTex_[0] = new Texture(width, height, format, input_format, type, 0, 0, 0, 0, 0, 0, report);
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

    setName(QString("fbo%1").arg(fbo_count_++));
}

void FrameBufferObject::addColorTextures(uint num)
{
    for (uint i=0; i<num; ++i)
    {
        auto tex = depth() < 2
                ? new Texture(width(), height(),
                               colorTex_[0]->format(), colorTex_[0]->format(),
                               colorTex_[0]->type(), 0, rep_)
                : new Texture(width(), height(), depth(),
                            colorTex_[0]->format(), colorTex_[0]->format(),
                            colorTex_[0]->type(), 0, rep_);
        colorTex_.push_back( tex );
    }
}

void FrameBufferObject::setName(const QString &name)
{
    name_ = name;
    for (uint i=0; i<colorTex_.size(); ++i)
    if (colorTex_[i])
        colorTex_[i]->setName(name + "_color" + QString::number(i));//_" + colorTex_->name());
    if (depthTex_)
        depthTex_->setName(name + "_depth_");// + depthTex_->name());
}

void FrameBufferObject::setChanged()
{
    for (auto tex : colorTex_)
        if (tex)
            tex->setChanged();
    if (depthTex_)
        depthTex_->setChanged();
}

FrameBufferObject::~FrameBufferObject()
{
    MO_DEBUG_GL("FrameBufferObject::~FrameBufferObject()");

    if (isCreated())
        MO_GL_WARNING("destructor of unreleased FrameBufferObject");

    delete depthTex_;
    for (auto tex : colorTex_)
        delete tex;
}


uint FrameBufferObject::width() const
{
    return !colorTex_.empty() && colorTex_[0]
            ? colorTex_[0]->width() : 0;
}

uint FrameBufferObject::height() const
{
    return !colorTex_.empty() && colorTex_[0]
            ? colorTex_[0]->height() : 0;
}


uint FrameBufferObject::depth() const
{
    return !colorTex_.empty() && colorTex_[0] && colorTex_[0]->is3d()
            ? colorTex_[0]->depth() : 1;
}

void FrameBufferObject::setViewport() const
{
    MO_DEBUG_GL("FrameBufferObject::setViewport()")
    MO_CHECK_GL_COND(rep_, glViewport(0,0, width(), height()) );
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

    for (auto t : colorTex_)
        if (t->isHandle())
            t->release();

    if (depthTex_ && depthTex_->isHandle())
        depthTex_->release();
}

bool FrameBufferObject::create()
{
    MO_DEBUG_GL("FrameBufferObject::create()");

    // release previous
    if (fbo_ != invalidGl)
        release_();

    // check for size setting
    if (colorTex_.empty() || !colorTex_[0]
        || colorTex_[0]->width() == 0 || colorTex_[0]->height() == 0)
    {
        MO_GL_ERROR_COND(rep_, "No size given for frame buffer object creation");
        return false;
    }

    for (auto t : colorTex_)
    if (!t->isHandle())
    {
        if (!t->create())
        {
            MO_GL_ERROR_COND(rep_, "could not create colorbuffer for framebuffer");
            return false;
        }
    }

    if (depthTex_ && !depthTex_->isHandle())
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

    GLenum target = cubemap_? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : GL_TEXTURE_2D;

    // attach color textures
    int k=0;
    for (auto tex : colorTex_)
    {
        if (!tex->is3d())
        {
            MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k++, target, tex->handle(), 0), err );
        }
        else
        {
            MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture3D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k++, target, tex->handle(), 0, 0), err );
        }
        if (err != GL_NO_ERROR) return false;
    }

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
        // XXX Should not do it automatically
        MO_CHECK_GL_RET_COND(rep_, glRenderbufferStorage(
                GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width(), height() ), err );
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

Texture * FrameBufferObject::swapColorTexture(Texture * tex, uint index)
{
    // change attachment
    GLenum
        err,
        target = cubemap_? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : GL_TEXTURE_2D;

    if (!tex->is3d())
    {
        MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex->handle(), 0), err );
    }
    else
    {
        MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture3D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex->handle(), 0, 0), err );
    }
    if (err != GL_NO_ERROR) return 0;

    std::swap(colorTex_[index], tex);
    return tex;
}

bool FrameBufferObject::attachCubeTexture(gl::GLenum target, uint i)
{
    if (!cubemap_)
    {
        MO_GL_ERROR_COND(rep_, "FrameBufferObject::attachCubeTexture() for non-cube texture");
        return false;
    }

    if (!colorTex_[i] || !colorTex_[i]->isAllocated())
    {
        MO_GL_ERROR_COND(rep_, "FrameBufferObject::attachCubeTexture() on uninitialized cube-map");
        return false;
    }

    GLenum err;
    MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, colorTex_[i]->handle(), 0), err );
    if (err != GL_NO_ERROR)
        return false;
    if (attachments_ & A_DEPTH)
    {
        MO_CHECK_GL_RET_COND(rep_, glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTex_->handle(), 0), err );
    }
    return err == GL_NO_ERROR;

}

bool FrameBufferObject::downloadColorTexture(void *ptr, uint index)
{
    return colorTex_[index]->download(ptr);
}

Texture * FrameBufferObject::takeColorTexture(uint index)
{
    Texture * t = colorTex_[index];

    // create new one
    if (!cubemap_)
        colorTex_[index] = new Texture(width(), height(),
                                       colorTex_[index]->format(), colorTex_[index]->format(),
                                       colorTex_[index]->type(), 0, rep_);
    else
    {
        colorTex_[index] = new Texture(width(), height(),
                                       colorTex_[index]->format(), colorTex_[index]->format(),
                                       colorTex_[index]->type(),
                                0, 0, 0, 0, 0, 0, rep_);
    }
    setName(name());

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

    if (attachments_ & A_DEPTH)
    {
        // create new one
        if (!cubemap_)
            depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(),
                                    colorTex_[0]->type(), 0, rep_);
        else
        {
            depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(), colorTex_[0]->type(),
                                    0, 0, 0, 0, 0, 0, rep_);
        }
        setName(name());
    }

    return t;
}

} // namespace GL
} // namespace MO
