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
#include "io/log_gl.h"
#include "io/streamoperators_glbinding.h"

using namespace gl;

namespace MO {
namespace GL {

namespace { static std::atomic_uint_fast64_t fbo_count_(0); }

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            bool cubemap, GLsizei multiSample)
    : FrameBufferObject(width, height, 1, format, GL::inputFormat(format), type, 0, cubemap, multiSample)
{

}

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum in_format, gl::GLenum type,
            bool cubemap, GLsizei multiSample)
    : FrameBufferObject(width, height, 1, format, in_format, type, 0, cubemap, multiSample)
{

}

FrameBufferObject::FrameBufferObject(gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum type,
            int attachmentMask, bool cubemap, GLsizei multiSample)
    : FrameBufferObject(width, height, 1, format, GL::inputFormat(format), type, attachmentMask, cubemap, multiSample)
{

}

FrameBufferObject::FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            int attachmentMask, bool cubemap, GLsizei multiSample)
    : FrameBufferObject(width, height, 1, format, input_format, type, attachmentMask,
                        cubemap, multiSample)
{

}

FrameBufferObject::FrameBufferObject(
            gl::GLsizei width, gl::GLsizei height, gl::GLsizei depth,
            gl::GLenum format, gl::GLenum input_format, gl::GLenum type,
            int attachmentMask, bool cubemap, GLsizei multiSample)
    : colorTex_         (0)
    , depthTex_         (0)
    , fbo_              (invalidGl)
    , rbo_              (invalidGl)
    , attachments_      (attachmentMask)
    , isCubemap_        (cubemap)
    , multiSamples_     (multiSample)
{
    MO_DEBUG_GL("FrameBufferObject::FrameBufferObject("
                << width << "x" << height << "x" << depth
                << ", " << format << ", " << type
                << ", " << cubemap << ", " << multiSample << ")");

    colorTex_.resize(1);
    if (!isCubemap_)
    {
        // 2d and 2d-multisample
        if (depth < 2)
            colorTex_[0] = new Texture(width, height, format, input_format, type, 0, multiSamples_);
        else
        // 3d
            colorTex_[0] = new Texture(width, height, depth, format, input_format, type, 0);
    }
    else
    {
        // cube-map
        colorTex_[0] = new Texture(width, height, format, input_format, type, 0, 0, 0, 0, 0, 0);
    }

    // depth attachment
    if (attachments_ & A_DEPTH)
    {
        if (!isCubemap_)
            depthTex_ = new Texture(
                width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0, multiSamples_);
        else
        {
            depthTex_ = new Texture(
                width, height, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, 0, 0, 0, 0, 0, 0);
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
                               colorTex_[0]->type(), 0)
                : new Texture(width(), height(), depth(),
                            colorTex_[0]->format(), colorTex_[0]->format(),
                            colorTex_[0]->type(), 0);
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
        depthTex_->setName(name + "_depth");// + depthTex_->name());
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
        MO_GL_WARNING("destructor of unreleased FrameBufferObject - OpenGL resource leak");

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

GLenum FrameBufferObject::type() const
{
    return !colorTex_.empty() && colorTex_[0]
            ? colorTex_[0]->type() : GL_FLOAT;
}

GLenum FrameBufferObject::format() const
{
    return !colorTex_.empty() && colorTex_[0]
            ? colorTex_[0]->format() : GL_RGBA;
}

void FrameBufferObject::setViewport() const
{
    MO_DEBUG_GL("FrameBufferObject::setViewport()")
    MO_CHECK_GL_THROW( glViewport(0,0, width(), height()) );
}


void FrameBufferObject::bind()
{
    MO_CHECK_GL_THROW( glBindFramebuffer(GL_FRAMEBUFFER, fbo_) );
    MO_CHECK_GL_THROW( glBindRenderbuffer(GL_RENDERBUFFER, rbo_) );
}

void FrameBufferObject::unbind()
{
    MO_CHECK_GL_THROW( glBindFramebuffer(GL_FRAMEBUFFER, 0) );
    MO_CHECK_GL_THROW( glBindRenderbuffer(GL_RENDERBUFFER, 0) );
}

void FrameBufferObject::release_()
{
    MO_DEBUG_GL("FrameBufferObject::release_()");

    MO_CHECK_GL_THROW( glDeleteRenderbuffers(1, &rbo_) );
    MO_CHECK_GL_THROW( glDeleteFramebuffers(1, &fbo_) );
    fbo_ = invalidGl;

    for (auto t : colorTex_)
        if (t->isHandle())
            t->release();

    if (depthTex_ && depthTex_->isHandle())
        depthTex_->release();
}

void FrameBufferObject::create()
{
    MO_DEBUG_GL("FrameBufferObject::create()");

    // release previous
    if (fbo_ != invalidGl)
        release_();

    // check for size setting
    if (colorTex_.empty() || !colorTex_[0]
        || colorTex_[0]->width() == 0 || colorTex_[0]->height() == 0)
    {
        MO_GL_ERROR("No size given for frame buffer object creation");
    }

    // create color textures
    for (auto t : colorTex_)
    if (!t->isHandle())
    {
        MO_EXTEND_EXCEPTION( t->create()
                             , "could not create colorbuffer for framebuffer" );
    }

    // create depth texture
    if (depthTex_ && !depthTex_->isHandle())
    {
        MO_EXTEND_EXCEPTION( depthTex_->create()
                             , "could not create depthbuffer for framebuffer" );
    }

    MO_CHECK_GL_THROW( glGenFramebuffers(1, &fbo_) );
    MO_CHECK_GL_THROW( glBindFramebuffer(GL_FRAMEBUFFER, fbo_) );

    // 2d texture or cubemap
    GLenum target = isCubemap_
                        ? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
                        : multiSamples_ > 0
                          ? GL_TEXTURE_2D_MULTISAMPLE
                          : GL_TEXTURE_2D;

    // attach color textures
    int k=0;
    for (auto tex : colorTex_)
    {
        if (!tex->is3d())
        {
            MO_CHECK_GL_THROW( glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k++, target, tex->handle(), 0) );
        }
        else
        {
            MO_CHECK_GL_THROW( glFramebufferTexture3D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + k++, GL_TEXTURE_3D, tex->handle(), 0, 0) );
        }
    }

    // for depth testing
    MO_CHECK_GL_THROW( glGenRenderbuffers(1, &rbo_) );
    MO_CHECK_GL_THROW( glBindRenderbuffer(GL_RENDERBUFFER, rbo_) );
    if (attachments_ & A_DEPTH)
    {
        // attach depth texture
        MO_CHECK_GL_THROW( glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTex_->handle(), 0) );
    }
    else
    {
        // attach internal storage for depth
        // XXX Should not do it automatically
        if (!multiSamples_)
            MO_CHECK_GL_THROW( glRenderbufferStorage(
                GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width(), height()) )
        else
            MO_CHECK_GL_THROW( glRenderbufferStorageMultisample(
                GL_RENDERBUFFER, multiSamples_, GL_DEPTH_COMPONENT24, width(), height()) );

        MO_CHECK_GL_THROW( glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_) );
    }

    MO_DEBUG_GL("FrameBufferObject::create()")
    MO_CHECK_GL_THROW( glViewport(0, 0, width(), height()) );
    MO_CHECK_GL_THROW( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
}

void FrameBufferObject::release()
{
    release_();
}

Texture * FrameBufferObject::swapColorTexture(Texture * tex, uint index)
{
    // change attachment
    GLenum
        target = isCubemap_? GL_TEXTURE_CUBE_MAP_NEGATIVE_Z : GL_TEXTURE_2D;

    if (!tex->is3d())
    {
        MO_CHECK_GL_THROW( glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex->handle(), 0) );
    }
    else
    {
        MO_CHECK_GL_THROW( glFramebufferTexture3D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex->handle(), 0, 0) );
    }

    std::swap(colorTex_[index], tex);
    return tex;
}

void FrameBufferObject::attachCubeTexture(gl::GLenum target, uint i)
{
    if (!isCubemap_)
    {
        MO_GL_ERROR("FrameBufferObject::attachCubeTexture() for non-cube texture");
    }

    if (!colorTex_[i] || !colorTex_[i]->isAllocated())
    {
        MO_GL_ERROR("FrameBufferObject::attachCubeTexture() on uninitialized cube-map");
    }

    MO_CHECK_GL_THROW( glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, colorTex_[i]->handle(), 0) );

    if (attachments_ & A_DEPTH)
    {
        MO_CHECK_GL_THROW( glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthTex_->handle(), 0) );
    }
}

void FrameBufferObject::downloadColorTexture(void *ptr, uint index)
{
    if (index >= colorTex_.size()
            || !colorTex_[index])
        MO_GL_ERROR("invalid texture download for '" << name() << "'");
    colorTex_[index]->download(ptr);
}

Texture * FrameBufferObject::takeColorTexture(uint index)
{
    Texture * t = colorTex_[index];

    // create new one
    if (!isCubemap_)
        colorTex_[index] = new Texture(width(), height(),
                                       colorTex_[index]->format(), colorTex_[index]->format(),
                                       colorTex_[index]->type(), 0, colorTex_[index]->isMultiSample());
    else
    {
        colorTex_[index] = new Texture(width(), height(),
                                       colorTex_[index]->format(), colorTex_[index]->format(),
                                       colorTex_[index]->type(),
                                       0, 0, 0, 0, 0, 0);
    }
    setName(name());

    return t;
}

void FrameBufferObject::downloadDepthTexture(void *ptr)
{
    if (!depthTex_)
        MO_GL_ERROR("invalid texture download for '" << name() << "'");
    depthTex_->download(ptr);
}

Texture * FrameBufferObject::takeDepthTexture()
{
    if (!depthTex_)
        return 0;

    Texture * t = depthTex_;

    if (attachments_ & A_DEPTH)
    {
        // create new one
        if (!isCubemap_)
            depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(),
                                    colorTex_[0]->type(), 0);
        else
        {
            depthTex_ = new Texture(width(), height(), depthTex_->format(), depthTex_->format(), colorTex_[0]->type(),
                                    0, 0, 0, 0, 0, 0);
        }
        setName(name());
    }

    return t;
}

} // namespace GL
} // namespace MO
