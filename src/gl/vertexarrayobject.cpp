/** @file vertexarrayobject.cpp

    @brief VertexArrayObject wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <memory>

#include "vertexarrayobject.h"
#include "bufferobject.h"
#include "gl_state.h"
#include "io/error.h"
#include "io/log.h"

using namespace gl;

namespace MO {
namespace GL {

/** A structure holding some additional infos to the BufferObject.
    Used for attributes and element buffers as well */
struct VertexArrayObject::Buffer_
{
    BufferObject * buf;

    int attribute;
    GLenum valueType, primitiveType;
    GLuint numVertices,
           attribLocation;
};




VertexArrayObject::VertexArrayObject(const QString& name)
    : name_             (name),
      vao_              (invalidGl)
{
}

VertexArrayObject::~VertexArrayObject()
{
    if (isCreated())
        MO_GL_WARNING("destruction of unreleased vertex array object '" << name_ << "'");
}

gl::GLuint VertexArrayObject::numVertices(uint index) const
{
    return index < elementBuffers_.size() ? elementBuffers_[index].numVertices : 0;
}

void VertexArrayObject::create()
{
    if (isCreated())
    {
        MO_GL_ERROR("Vertex array object '" << name_ << "' already created");
    }

    clear();

    MO_CHECK_GL_THROW( glGenVertexArrays(1, &vao_) );
}

void VertexArrayObject::release()
{
    if (!isCreated())
        MO_GL_WARNING("release on uninitialized vertex array object '" << name_ << "'");

    MO_CHECK_GL( glDeleteVertexArrays(1, &vao_) );
    vao_ = invalidGl;    

    clear();
}

void VertexArrayObject::clear()
{
    for (auto &b : buffers_)
    {
        b.second.buf->release();
        delete b.second.buf;
    }
    buffers_.clear();

    for (auto &b : elementBuffers_)
    {
        b.buf->release();
        delete b.buf;
    }
    elementBuffers_.clear();
}

BufferObject * VertexArrayObject::getAttributeBufferObject(int a)
{
    auto i = buffers_.find(a);
    if (i != buffers_.end())
        return i->second.buf;

    return 0;
}

BufferObject * VertexArrayObject::createAttribBuffer(
        int attribute,
        GLuint location, GLenum valueType, GLint numberCoordinates,
        GLuint sizeInBytes, const void * ptr,
        GLenum storageType, GLint stride, GLboolean normalized)
{
    BufferObject * buf = 0;

    if (!isCreated())
    {
        MO_GL_ERROR( "createAttribBuffer() on uninitialized vertex array object '" << name_ << "'");
    }

    if (getAttributeBufferObject(attribute))
    {
        MO_GL_ERROR( "createAttributeBuffer() for duplicate attribute " << attribute);
    }

    buf = new BufferObject(name_ + "_attrib");
    std::unique_ptr<BufferObject*> bufDeleter;

    buf->create(GL_ARRAY_BUFFER, storageType);
    try
    {
        buf->bind();
        buf->upload(ptr, sizeInBytes);

        MO_CHECK_GL_THROW( glEnableVertexAttribArray(location) );

        //MO_DEBUG("glVertexAttribPointer("<<location<<", "<<numberCoordinates
        //         <<", "<<valueType<<", "<<(int)normalized<<", "<<stride<<", "<<0<<")");
        MO_CHECK_GL_THROW( glVertexAttribPointer(
                location, numberCoordinates, valueType, normalized, stride, NULL) );

        // keep track
        Buffer_ b;
        b.attribute = attribute;
        b.buf = buf;
        b.attribLocation = location;
        buffers_.insert( std::make_pair(attribute, b) );
    }
    catch (Exception& e)
    {
        buf->release();
        throw;
    }

    bufDeleter.release();

    return buf;
}


BufferObject * VertexArrayObject::createIndexBuffer(
        GLenum primitiveType,
        GLenum valueType,
        GLuint numberVertices,
        const void * ptr,
        GLenum storageType)
{
    if (!isCreated())
    {
        MO_GL_ERROR( "createIndexBuffer() on uninitialized vertex array object '" << name_ << "'");
        return 0;
    }

    BufferObject * buf = new BufferObject(name_ + "_idx");
    // temporarily bind for deletion
    std::unique_ptr<BufferObject> bufDeleter(buf);

    buf->create(GL_ELEMENT_ARRAY_BUFFER, storageType);

    try
    {
        buf->bind();
        buf->upload(ptr, numberVertices * typeSize(valueType));

        // create new storage instance
        Buffer_ buffer;

        // keep track
        buffer.buf = buf;
        buffer.valueType = valueType;
        buffer.numVertices = numberVertices;
        buffer.primitiveType = primitiveType;
        buffer.attribute = 0;
        buffer.attribLocation = 0;
        elementBuffers_.push_back(buffer);
    }
    catch (Exception& e)
    {
        buf->release();
        throw;
    }

    // all ok, don't delete the buffer
    bufDeleter.release();

    return buf;
}

#ifndef MO_USE_OPENGL_CORE
BufferObject * VertexArrayObject::createEdgeFlagBuffer(
        GLuint sizeInBytes, const void * ptr,
        GLenum storageType, GLint stride)
{
    BufferObject * buf = 0;

    if (!isCreated())
    {
        MO_GL_ERROR_COND(rep_, "createEdgeFlagBuffer() on uninitialized vertex array object '" << name_ << "'");
        return 0;
    }

    buf = new BufferObject(name_ + "_edge");
    std::unique_ptr<BufferObject*> bufDeleter;

    buf->create(GL_ARRAY_BUFFER, storageType);

    try
    {
        buf->bind();
        buf->upload(ptr, sizeInBytes);

        MO_CHECK_GL_THROW( glEnableClientState(GL_EDGE_FLAG_ARRAY) );

        MO_CHECK_GL_THROW( glEdgeFlagPointer(stride, NULL) );

        // keep track
        Buffer_ b;
        b.attribute = 0;
        b.attribLocation = 0;
        b.buf = buf;
        edgeFlagBuffers_.push_back(b);
    }
    catch (Exception& e)
    {
        buf->release();
        throw;
    }

    bufDeleter.release();

    return buf;
}
#endif

void VertexArrayObject::bind()
{
    MO_CHECK_GL_THROW( glBindVertexArray(vao_) );
}

void VertexArrayObject::unbind()
{
    MO_CHECK_GL_THROW( glBindVertexArray(0) );
}


void VertexArrayObject::drawElements(int instanceCount) const
{
    if (elementBuffers_.empty())
    {
        MO_GL_ERROR( "No element buffers defined for "
                     "VertexArrayObject('" << name_ << "')::drawElements()");
    }

    try
    {
        // bind the vao;
        MO_CHECK_GL_THROW( glBindVertexArray(vao_) );

        // for each element buffer
        for (auto & ebuffer : elementBuffers_)
        {
            // bind element buffer
            ebuffer.buf->bind();

            //MO_PRINT(GlState::current().toString());
            if (instanceCount <= 1)
                MO_CHECK_GL_THROW( glDrawElements(
                                        ebuffer.primitiveType,
                                        ebuffer.numVertices,
                                        ebuffer.valueType,
                                        (void*)0) )
            else
                MO_CHECK_GL_THROW( glDrawElementsInstanced(
                                        ebuffer.primitiveType,
                                        ebuffer.numVertices,
                                        ebuffer.valueType,
                                        (void*)0,
                                        instanceCount) );

        }
    }
    catch (Exception& e)
    {
        MO_CHECK_GL_THROW(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        MO_CHECK_GL( glBindVertexArray(0) );
        e << "\n  in VertexArrayObject::drawElements(" << instanceCount << ")";
        throw;
    }

    // unbind
    MO_CHECK_GL_THROW(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL_THROW(glBindVertexArray(0));
}


void VertexArrayObject::drawElements(uint eIndex,
        gl::GLenum primitiveType, gl::GLuint numberVertices, gl::GLuint offset) const
{
    if (eIndex >= elementBuffers_.size())
    {
        MO_GL_ERROR( "element buffer index " << eIndex << " out of range for "
                     "VertexArrayObject('" << name_ << "')::drawElements()");
    }

    if (numberVertices <= 0)
        numberVertices = elementBuffers_[eIndex].numVertices;

    try
    {
        MO_CHECK_GL_THROW( glBindVertexArray(vao_) );

        // Some drivers don't seem to store the element array in the vao state!!
        // http://stackoverflow.com/questions/8973690/vao-and-element-array-buffer-state
        elementBuffers_[eIndex].buf->bind();

        //MO_DEBUG("glDrawElements("<<primitiveType<<", "<<numberVertices<<", "<<elementBuffer_->valueType
        //         <<", "<<reinterpret_cast<void*>(offset)<<")");
        MO_CHECK_GL_THROW( glDrawElements(
                                    primitiveType,
                                    numberVertices,
                                    elementBuffers_[eIndex].valueType,
                                    reinterpret_cast<void*>(offset)) );
    }
    catch (Exception& e)
    {
        MO_CHECK_GL( glBindVertexArray(0) );
        throw;
    }

    MO_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL(glBindVertexArray(0));
}


} // namespace GL
} // namespace MO
