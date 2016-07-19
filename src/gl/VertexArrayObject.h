/** @file vertexarrayobject.h

    @brief VertexArrayObject wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_VERTEXARRAYOBJECT_H
#define MOSRC_GL_VERTEXARRAYOBJECT_H

#include <map>

#include <QString>

#include "opengl.h"

namespace MO {
namespace GL {

class BufferObject;

class VertexArrayObject
{
public:
    enum Attribute
    {
        A_POSITION = 1,
        A_COLOR,
        A_NORMAL,
        A_TEX_COORD,
        A_USER = 100
    };

    /** @p name is for debugging purposes */
    explicit VertexArrayObject(const QString& name);
    ~VertexArrayObject();

    // --------- getter -----------

    /** Returns the openGL name of the vertex array object */
    gl::GLuint id() const { return vao_; }

    /** Returns true, when a vertex array object has been created */
    bool isCreated() const { return vao_ != invalidGl; }

    /** Returns the attribute buffer object for the given attribute (Attribute enum), or NULL */
    BufferObject * getAttributeBufferObject(int a);

    /** Returns the number of vertices as defined in the element buffers.
        @p index represents the xth element buffer added with createIndexBuffer().
        If @p index is out of range, 0 will be returned. */
    gl::GLuint numVertices(uint index = 0) const;


    // --------- opengl -----------

    /** Creates a vertex array object.
        @throws GlException */
    void create();
    /** Releases the vertex array object and all buffers */
    void release();
    /** Clears and releases the associated buffers */
    void clear();

    /** Binds the vertex array object (not needed for the draw methods).
        @throws GlException */
    void bind();
    /** Unbinds the vertex array object (not needed for the draw methods).
        @throws GlException */
    void unbind();

    /** Creates a vertex attribute array buffer.
        Returns the BufferObject, or NULL on failure.
        The vertex array object needs to be bound.
        @p attributeType is one of the Attribute enums or A_USER + N.
        @throws GlException */
    BufferObject * createAttribBuffer(
            int attributeType,
            gl::GLuint attributeLocation, gl::GLenum valueType, gl::GLint numberCoordinates,
            gl::GLuint sizeInBytes, const void * ptr,
            gl::GLenum storageType = gl::GL_STATIC_DRAW, gl::GLint stride = 0,
            gl::GLboolean normalized = gl::GL_FALSE);

    /** Creates an element array buffer.
        You can create multiple element buffers with multiple calls.
        Returns the BufferObject, or NULL on failure.
        The vertex array object needs to be bound.
        @throws GlException */
    BufferObject * createIndexBuffer(
            gl::GLenum primitiveType,
            gl::GLenum valueType,
            gl::GLuint numberVertices, const void * ptr,
            gl::GLenum storageType = gl::GL_STATIC_DRAW);

#ifndef MO_USE_OPENGL_CORE
    /** Creates an edge flag buffer.
        Returns the BufferObject, or NULL on failure.
        The vertex array object needs to be bound.
        @throws GlException */
    BufferObject * createEdgeFlagBuffer(
            gl::GLuint sizeInBytes, const void * ptr,
            gl::GLenum storageType = gl::GL_STATIC_DRAW,
            gl::GLint stride = 0);
#endif

    /** Consecutively draws all element arrays added with createIndexBuffer().
        If @p instanceCount > 1, a couple of instances will be drawn and
        the built-in variable gl_InstanceID will reflect the instance number.
        @throws GlException */
    void drawElements(int instanceCount = 1) const;

    /** Draws the vertex array object and overrides the settings from createIndexBuffer().
        If @p numberVertices <= 0, the number from createIndexBuffer() is used.
        Objects are unbound on return.
        @throws GlException */
    void drawElements(uint elementBufferIndex,
                      gl::GLenum primitiveType, gl::GLuint numberVertices = 0,
                      gl::GLuint offset = 0) const;

private:

    QString name_;

    gl::GLuint vao_;

    struct Buffer_;
    std::map<int, Buffer_> buffers_;
    /** Buffer for each index type (e.g. triangles or lines) */
    std::vector<Buffer_> elementBuffers_;
    std::vector<Buffer_> edgeFlagBuffers_;
};

} // namespace GL
} // namespace MO


#endif // MOSRC_GL_VERTEXARRAYOBJECT_H
