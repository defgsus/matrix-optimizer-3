/** @file bufferobject.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include "bufferobject.h"


namespace MO {
namespace GL {

using namespace ::gl;

BufferObject::BufferObject(const QString& name, ErrorReporting rep)
    : p_handle_     (invalidGl),
      p_size_       (0),
      p_name_       (name),
      p_rep_        (rep)
{
}

BufferObject::~BufferObject()
{
    if (p_handle_ != invalidGl)
        MO_GL_WARNING("Destructor of unreleased BufferObject " << p_name_);
}

bool BufferObject::create(GLenum target, GLenum storageType)
{
    // release previous?
    if (isOk())
        release();

    GLenum e;
    MO_CHECK_GL_RET_COND(p_rep_, glGenBuffers(1, &p_handle_), e);
    if (e != GL_NO_ERROR) return (p_handle_ = invalidGl);

    p_target_ = target;
    p_storage_ = storageType;
    return true;
}


#define MO__CHECK_OK(func__)                                                                        \
    if (!isOk())                                                                                    \
    {                                                                                               \
        MO_GL_ERROR_COND(p_rep_, "BufferObject::" << func__ << " on uninitialized buffer object");  \
        return false;                                                                               \
    }

bool BufferObject::release()
{
    MO__CHECK_OK("release()");

    GLenum e;
    MO_CHECK_GL_RET_COND(p_rep_, glDeleteBuffers(1, &p_handle_), e );
    if (e != GL_NO_ERROR) return false;

    p_handle_ = invalidGl;
    return true;
}

bool BufferObject::bind()
{
    MO__CHECK_OK("bind()");

    GLenum e;
    MO_CHECK_GL_RET_COND(p_rep_, glBindBuffer(p_target_, p_handle_), e);
    if (e != GL_NO_ERROR) return false;

    return true;
}

bool BufferObject::unbind()
{
    MO__CHECK_OK("unbind()");

    GLenum e;
    MO_CHECK_GL_RET_COND(p_rep_, glBindBuffer(p_target_, 0), e);
    if (e != GL_NO_ERROR) return false;

    return true;
}

bool BufferObject::upload(const void* ptr, gl::GLsizeiptr sizeInBytes)
{
    p_size_ = sizeInBytes;
    return upload(ptr);
}

bool BufferObject::upload(const void* ptr, gl::GLenum storage)
{
    p_storage_ = storage;
    return upload(ptr);
}

bool BufferObject::upload(const void* ptr)
{
    MO__CHECK_OK("upload()");

    GLenum e;
    MO_CHECK_GL_RET_COND(p_rep_, glBufferData(p_target_,
                                            p_size_, ptr, p_storage_), e);
    if (e != GL_NO_ERROR) return false;

    return true;
}



} // namespace GL
} // namespace MO
