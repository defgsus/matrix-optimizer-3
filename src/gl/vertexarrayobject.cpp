/** @file vertexarrayobject.cpp

    @brief VertexArrayObject wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <memory>

#include "vertexarrayobject.h"
#include "bufferobject.h"
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




VertexArrayObject::VertexArrayObject(const QString& name, ErrorReporting errorReport)
    : name_         (name),
      rep_          (errorReport),
      vao_          (invalidGl)
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

bool VertexArrayObject::create()
{
    if (isCreated())
    {
        MO_GL_ERROR_COND(rep_, "Vertex array object '" << name_ << "' already created");
        return false;
    }

    clear();

    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glGenVertexArrays(1, &vao_), e );
    if (e != GL_NO_ERROR) return false;

    return true;
}

void VertexArrayObject::release()
{
    if (!isCreated())
        MO_GL_ERROR_COND(rep_, "release on uninitialized vertex array object '" << name_ << "'");

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
        MO_GL_ERROR_COND(rep_, "createAttribBuffer() on uninitialized vertex array object '" << name_ << "'");
        return 0;
    }

    if (getAttributeBufferObject(attribute))
    {
        MO_GL_ERROR_COND(rep_, "createAttributeBuffer() for duplicate attribute " << attribute);
        return 0;
    }

    GLenum e;

    buf = new BufferObject(name_ + "_attrib", rep_);
    std::unique_ptr<BufferObject*> bufDeleter;

    if (!buf->create(GL_ARRAY_BUFFER, storageType))
        return 0;

    if (!buf->bind())
    {
        buf->release();
        return 0;
    }

    if (!buf->upload(ptr, sizeInBytes))
    {
        buf->release();
        return 0;
    }

    MO_CHECK_GL_RET_COND(rep_, glEnableVertexAttribArray(location), e);
    if (e != GL_NO_ERROR)
    {
        buf->release();
        return 0;
    }
    //MO_DEBUG("glVertexAttribPointer("<<location<<", "<<numberCoordinates
    //         <<", "<<valueType<<", "<<(int)normalized<<", "<<stride<<", "<<0<<")");
    MO_CHECK_GL_RET_COND(rep_, glVertexAttribPointer(
            location, numberCoordinates, valueType, normalized, stride, NULL), e);
    if (e != GL_NO_ERROR)
    {
        buf->release();
        return 0;
    }

    // keep track
    Buffer_ b;
    b.attribute = attribute;
    b.buf = buf;
    b.attribLocation = location;
    buffers_.insert( std::make_pair(attribute, b) );

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
        MO_GL_ERROR_COND(rep_, "createIndexBuffer() on uninitialized vertex array object '" << name_ << "'");
        return 0;
    }

    BufferObject * buf = new BufferObject(name_ + "_idx", rep_);
    // temporarily bind for deletion
    std::unique_ptr<BufferObject> bufDeleter(buf);

    if (!buf->create(GL_ELEMENT_ARRAY_BUFFER, storageType))
        return 0;

    if (!buf->bind())
    {
        buf->release();
        return 0;
    }

    if (!buf->upload(ptr, numberVertices * typeSize(valueType)))
    {
        buf->release();
        return 0;
    }

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

    // all ok, don't delete the buffer
    bufDeleter.release();

    return buf;
}

bool VertexArrayObject::bind()
{
    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e != GL_NO_ERROR) { return false; }


    return true;
}

void VertexArrayObject::unbind()
{
    MO_CHECK_GL_COND(rep_, glBindVertexArray(0));
}


bool VertexArrayObject::drawElements(int instanceCount) const
{
    if (elementBuffers_.empty())
    {
        MO_GL_ERROR_COND(rep_, "No element buffers defined for "
                         "VertexArrayObject('" << name_ << "')::drawElements()");
        return false;
    }

    // bind the vao;
    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e != GL_NO_ERROR) { return false; }

    // for each element buffer
    for (auto & ebuffer : elementBuffers_)
    {
        if (!ebuffer.buf->bind())
        {
            MO_CHECK_GL(glBindVertexArray(0));
            return false;
        }

        if (instanceCount <= 1)
            MO_CHECK_GL_RET_COND(rep_, glDrawElements(
                                    ebuffer.primitiveType,
                                    ebuffer.numVertices,
                                    ebuffer.valueType,
                                    (void*)0), e)
        else
            MO_CHECK_GL_RET_COND(rep_, glDrawElementsInstanced(
                                    ebuffer.primitiveType,
                                    ebuffer.numVertices,
                                    ebuffer.valueType,
                                    (void*)0,
                                    instanceCount), e);

    }

    MO_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL(glBindVertexArray(0));

    return e != GL_NO_ERROR;
}


bool VertexArrayObject::drawElements(uint eIndex,
        gl::GLenum primitiveType, gl::GLuint numberVertices, gl::GLuint offset) const
{
    if (eIndex >= elementBuffers_.size())
    {
        MO_GL_ERROR_COND(rep_, "element buffer index " << eIndex << " out of range for "
                         "VertexArrayObject('" << name_ << "')::drawElements()");
        return false;
    }

    if (numberVertices <= 0)
        numberVertices = elementBuffers_[eIndex].numVertices;

    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e != GL_NO_ERROR) { return false; }

    // Some drivers don't seem to store the element array in the vao state!!
    // http://stackoverflow.com/questions/8973690/vao-and-element-array-buffer-state
    if (!elementBuffers_[eIndex].buf->bind())
    {
        MO_CHECK_GL(glBindVertexArray(0));
        return false;
    }

    //MO_DEBUG("glDrawElements("<<primitiveType<<", "<<numberVertices<<", "<<elementBuffer_->valueType
    //         <<", "<<reinterpret_cast<void*>(offset)<<")");
    MO_CHECK_GL_RET_COND(rep_, glDrawElements(
                                primitiveType,
                                numberVertices,
                                elementBuffers_[eIndex].valueType,
                                reinterpret_cast<void*>(offset)), e);

    MO_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL(glBindVertexArray(0));

    return e != GL_NO_ERROR;
}


} // namespace GL
} // namespace MO
