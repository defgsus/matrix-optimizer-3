/** @file vertexarrayobject.h

    @brief VertexArrayObject wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_VERTEXARRAYOBJECT_H
#define MOSRC_GL_VERTEXARRAYOBJECT_H

#include <vector>

#include <QString>

#include "opengl.h"

namespace MO {
namespace GL {

// XXX Probably more meaningful to create separate BufferObjects at some point
class VertexArrayObject
{
public:
    /** @p name is for debugging purposes */
    explicit VertexArrayObject(const QString& name, ErrorReporting = ER_THROW);
    ~VertexArrayObject();

    // --------- getter -----------

    /** Returns the openGL name of the vertex array object */
    gl::GLuint id() const { return vao_; }

    /** Returns true, when a vertex array object has been created */
    bool isCreated() const { return vao_ != invalidGl; }

    // --------- opengl -----------

    /** Creates a vertex array object */
    bool create();
    /** Releases the vertex array object and all buffers */
    void release();

    /** Binds the vertex array object (not needed for the draw methods) */
    bool bind();
    /** Unbinds the vertex array object (not needed for the draw methods) */
    void unbind();

    /** Creates a vertex attribute array buffer.
        Returns the opengl name of the buffer, or invalidGl on failure.
        The vertex array object needs to be bound. */
    gl::GLuint createAttribBuffer(
            gl::GLuint attributeLocation, gl::GLenum valueType, gl::GLint numberCoordinates,
            gl::GLuint sizeInBytes, const void * ptr,
            gl::GLenum storageType = gl::GL_STATIC_DRAW, gl::GLint stride = 0,
            gl::GLboolean normalized = gl::GL_FALSE);

    /** Creates an element array buffer.
        Returns the opengl name of the buffer, or invalidGl on failure.
        The vertex array object needs to be bound. */
    gl::GLuint createIndexBuffer(
            gl::GLenum valueType,
            gl::GLuint numberVertices, const void * ptr,
            gl::GLenum storageType = gl::GL_STATIC_DRAW);

    /** Draws the vertex array object.
        If @p numberVertices <= 0, the number from createIndexBuffer() is used.
        Objects are unbound on return. */
    bool drawElements(gl::GLenum primitiveType, gl::GLuint numberVertices = 0,
                      gl::GLuint offset = 0) const;

private:

    QString name_;

    ErrorReporting rep_;
    gl::GLuint vao_;

    struct Buffer_;
    std::vector<Buffer_> buffers_;
    Buffer_ * elementBuffer_;
};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_VERTEXARRAYOBJECT_H
