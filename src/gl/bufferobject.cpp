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

BufferObject::BufferObject(const QString& name)
    : p_handle_     (invalidGl),
      p_size_       (0),
      p_name_       (name)
{
}

BufferObject::~BufferObject()
{
    if (p_handle_ != invalidGl)
        MO_GL_WARNING("Destructor of unreleased BufferObject " << p_name_
                      << " - OpenGL resource leak");
}

void BufferObject::create(GLenum target, GLenum storageType)
{
    // release previous?
    if (isOk())
        release();

    MO_CHECK_GL_THROW( glGenBuffers(1, &p_handle_) );

    p_target_ = target;
    p_storage_ = storageType;
}


#define MO__CHECK_OK(func__)                                                           \
    if (!isOk())                                                                       \
    {                                                                                  \
        MO_GL_ERROR( "BufferObject::" << func__ << " on uninitialized buffer object"); \
        return;                                                                        \
    }

void BufferObject::release()
{
    MO__CHECK_OK("release()");

    MO_CHECK_GL_THROW( glDeleteBuffers(1, &p_handle_) );

    p_handle_ = invalidGl;
}

void BufferObject::bind()
{
    MO__CHECK_OK("bind()");

    MO_CHECK_GL_THROW( glBindBuffer(p_target_, p_handle_) );
}

void BufferObject::unbind()
{
    MO__CHECK_OK("unbind()");

    MO_CHECK_GL_THROW( glBindBuffer(p_target_, 0) );
}

void BufferObject::upload(const void* ptr, gl::GLsizeiptr sizeInBytes)
{
    p_size_ = sizeInBytes;
    upload(ptr);
}

void BufferObject::upload(const void* ptr, gl::GLenum storage)
{
    p_storage_ = storage;
    upload(ptr);
}

void BufferObject::upload(const void* ptr)
{
    MO__CHECK_OK("upload()");

    MO_CHECK_GL_THROW( glBufferData(p_target_, p_size_, ptr, p_storage_) );
}



} // namespace GL
} // namespace MO
