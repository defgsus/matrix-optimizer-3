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


class BufferObject
{
public:
    BufferObject(const QString& name, ErrorReporting rep);
    ~BufferObject();

    // ---------------- getter ------------------

    /** Returns native handle, or GL::invalidGl */
    gl::GLuint handle() const { return p_handle_; }

    gl::GLuint sizeInBytes() const { return p_size_; }

    bool isOk() const { return p_handle_ != invalidGl; }


    // ------------ opengl ----------------------

    /** @p target is one of GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.. */
    bool create(gl::GLenum target, gl::GLenum storageType);

    bool release();

    bool bind();
    bool unbind();

    /** Uploads the buffer data.
        Buffer must be bound. */
    bool upload(const void * ptr, gl::GLsizeiptr sizeInBytes);

    /** Uploads to previously defined size.
        Buffer must be bound. */
    bool upload(const void* ptr);

private:

    gl::GLuint p_handle_;
    gl::GLsizeiptr p_size_;
    gl::GLenum p_target_, p_storage_;
    QString p_name_;
    ErrorReporting p_rep_;
};


} // namespace GL
} // namespace MO


#endif // MOSRC_GL_BUFFEROBJECT_H
