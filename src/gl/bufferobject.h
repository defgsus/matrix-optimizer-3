/** @file bufferobject.h

    @brief Wrapper for openGL buffer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#ifndef MOSRC_GL_BUFFEROBJECT_H
#define MOSRC_GL_BUFFEROBJECT_H

#include <QString>

#include "opengl.h"

namespace MO {
namespace GL {


/** Wrapper around an OpenGL Buffer Object */
class BufferObject
{
public:
    BufferObject(const QString& name);
    ~BufferObject();

    // ---------------- getter ------------------

    /** Returns native handle, or GL::invalidGl */
    gl::GLuint handle() const { return p_handle_; }

    gl::GLuint sizeInBytes() const { return p_size_; }

    bool isOk() const { return p_handle_ != invalidGl; }


    // ------------ opengl ----------------------

    /** @p target is one of GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc..
        @throws GlException */
    void create(gl::GLenum target, gl::GLenum storageType);

    void release();

    /** @throws GlException */
    void bind();
    /** @throws GlException */
    void unbind();

    /** Uploads the given buffer data.
        Stores the pointer and size.
        Buffer must be bound.
        @throws GlException */
    void upload(const void * ptr, gl::GLsizeiptr sizeInBytes);

    /** Uploads to previously defined size with new pointer.
        Buffer must be bound.
        @throws GlException */
    void upload(const void* ptr);

    /** Uploads to previously defined size with new pointer and storage type.
        Buffer must be bound.
        @throws GlException */
    void upload(const void* ptr, gl::GLenum storageType);

private:

    gl::GLuint p_handle_;
    gl::GLsizeiptr p_size_;
    gl::GLenum p_target_, p_storage_;
    QString p_name_;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_BUFFEROBJECT_H
