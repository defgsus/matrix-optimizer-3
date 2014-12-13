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

struct VertexArrayObject::Buffer_
{
    BufferObject * buf;

    GLenum valueType;
    GLuint numVertices,
           attribLocation;
};




VertexArrayObject::VertexArrayObject(const QString& name, ErrorReporting errorReport)
    : name_         (name),
      rep_          (errorReport),
      vao_          (invalidGl),
      elementBuffer_(0)
{
}

VertexArrayObject::~VertexArrayObject()
{
    if (isCreated())
        MO_GL_WARNING("destruction of unreleased vertex array object '" << name_ << "'");

    delete elementBuffer_;
}

bool VertexArrayObject::create()
{
    if (isCreated())
    {
        MO_GL_ERROR_COND(rep_, "Vertex array object '" << name_ << "' already created");
        return false;
    }

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

    for (auto &b : buffers_)
    {
        b.buf->release();
        delete b.buf;
    }
    buffers_.clear();

    if (elementBuffer_)
    {
        elementBuffer_->buf->release();
        delete elementBuffer_;
        elementBuffer_ = 0;
    }

}


BufferObject * VertexArrayObject::createAttribBuffer(
        GLuint location, GLenum valueType, GLint numberCoordinates,
        GLuint sizeInBytes, const void * ptr,
        GLenum storageType, GLint stride, GLboolean normalized)
{
    BufferObject * buf = 0;

    if (!isCreated())
    {
        MO_GL_ERROR_COND(rep_, "createAttribBuffer() on uninitialized vertex array object '" << name_ << "'");
        return buf;
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
    b.buf = buf;
    b.attribLocation = location;
    buffers_.push_back(b);

    bufDeleter.release();

    return buf;
}


BufferObject * VertexArrayObject::createIndexBuffer(
        GLenum valueType,
        GLuint numberVertices, const void * ptr,
        GLenum storageType)
{
    BufferObject * buf = 0;

    if (!isCreated())
    {
        MO_GL_ERROR_COND(rep_, "createIndexBuffer() on uninitialized vertex array object '" << name_ << "'");
        return 0;
    }

    //GLenum e;

    buf = new BufferObject(name_ + "_idx", rep_);
    std::unique_ptr<BufferObject*> bufDeleter;

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

    // delete previous
    if (elementBuffer_)
    {
        elementBuffer_->buf->release();
    }
    else elementBuffer_ = new Buffer_;

    bufDeleter.release();

    // keep track
    elementBuffer_->buf = buf;
    elementBuffer_->valueType = valueType;
    elementBuffer_->numVertices = numberVertices;

    return buf;
}

bool VertexArrayObject::bind()
{
    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e != GL_NO_ERROR) { return false; }

    if (elementBuffer_)
    {
        if (!elementBuffer_->buf->bind())
            return false;
    }

    return true;
}

void VertexArrayObject::unbind()
{
    if (elementBuffer_)
        elementBuffer_->buf->unbind();
    MO_CHECK_GL_COND(rep_, glBindVertexArray(0));
}

bool VertexArrayObject::drawElements(
        GLenum primitiveType, GLuint numberVertices, GLuint offset) const
{
    if (!elementBuffer_)
    {
        MO_GL_ERROR_COND(rep_, "no element buffer defined for "
                         "VertexArrayObject('" << name_ << "')::drawElements()");
        return false;
    }

    if (numberVertices <= 0)
        numberVertices = elementBuffer_->numVertices;

    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e != GL_NO_ERROR) { return false; }

    // Some drivers don't seem to store the element array in the vao state!!
    // http://stackoverflow.com/questions/8973690/vao-and-element-array-buffer-state
    if (!elementBuffer_->buf->bind())
    {
        MO_CHECK_GL(glBindVertexArray(0));
        return false;
    }

    //MO_DEBUG("glDrawElements("<<primitiveType<<", "<<numberVertices<<", "<<elementBuffer_->valueType
    //         <<", "<<reinterpret_cast<void*>(offset)<<")");
    MO_CHECK_GL_RET_COND(rep_, glDrawElements(
                                primitiveType,
                                numberVertices,
                                elementBuffer_->valueType,
                                reinterpret_cast<void*>(offset)), e);

    MO_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL(glBindVertexArray(0));

    return e != GL_NO_ERROR;
}


} // namespace GL
} // namespace MO
