/** @file vertexarrayobject.cpp

    @brief VertexArrayObject wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include "vertexarrayobject.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GL {

struct VertexArrayObject::Buffer_
{
    GLuint id;
    GLenum target, valueType;
    GLuint numVertices;
};




VertexArrayObject::VertexArrayObject(ErrorReporting errorReport)
    : rep_          (errorReport),
      vao_          (invalidGl),
      elementBuffer_(0)
{
}

VertexArrayObject::~VertexArrayObject()
{
    if (isCreated())
        MO_GL_WARNING("destruction of unreleased vertex array object");

    delete elementBuffer_;
}

bool VertexArrayObject::create()
{
    if (isCreated())
    {
        MO_GL_ERROR_COND(rep_, "Vertex array object already created");
        return false;
    }

    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glGenVertexArrays(1, &vao_), e );
    if (e) return false;

    return true;
}

void VertexArrayObject::release()
{
    if (!isCreated())
        MO_GL_ERROR_COND(rep_, "release on uninitialized vertex array object");

    MO_CHECK_GL( glDeleteVertexArrays(1, &vao_) );
    vao_ = invalidGl;

    for (auto &b : buffers_)
        MO_CHECK_GL( glDeleteBuffers(1, &b.id) );
    buffers_.clear();

    if (elementBuffer_)
    {
        MO_CHECK_GL( glDeleteBuffers(1, &elementBuffer_->id) );
        delete elementBuffer_;
        elementBuffer_ = 0;
    }

}


GLuint VertexArrayObject::createAttribBuffer(
        GLuint location, GLenum valueType, GLint numberCoordinates,
        GLuint sizeInBytes, const void * ptr,
        GLenum storageType, GLint stride, GLboolean normalized)
{
    GLuint buf = invalidGl;

    if (!isCreated())
    {
        MO_GL_ERROR_COND(rep_, "createAttribBuffer() on uninitialized vertex array object");
        return buf;
    }

    GLenum e;

    MO_CHECK_GL_RET_COND(rep_, glGenBuffers(1, &buf), e);
    if (e) return invalidGl;

    MO_CHECK_GL_RET_COND(rep_, glBindBuffer(GL_ARRAY_BUFFER, buf), e);
    if (e) goto fail;

    MO_CHECK_GL_RET_COND(rep_, glBufferData(GL_ARRAY_BUFFER,
                    sizeInBytes, ptr, storageType), e);
    if (e) goto fail;

    MO_CHECK_GL_RET_COND(rep_, glEnableVertexAttribArray(location), e);
    if (e) goto fail;

    //MO_DEBUG("glVertexAttribPointer("<<location<<", "<<numberCoordinates
    //         <<", "<<valueType<<", "<<(int)normalized<<", "<<stride<<", "<<0<<")");
    MO_CHECK_GL_RET_COND(rep_, glVertexAttribPointer(
            location, numberCoordinates, valueType, normalized, stride, NULL), e);
    if (e) goto fail;

    // keep track
    Buffer_ b;
    b.id = buf;
    b.target = GL_ARRAY_BUFFER;
    buffers_.push_back(b);

    return buf;

fail:
    MO_CHECK_GL(glDeleteBuffers(1, &buf));
    return invalidGl;
}



GLuint VertexArrayObject::createIndexBuffer(
        GLenum valueType,
        GLuint numberVertices, const void * ptr,
        GLenum storageType)
{
    GLuint buf = invalidGl;

    if (!isCreated())
    {
        MO_GL_ERROR_COND(rep_, "createIndexBuffer() on uninitialized vertex array object");
        return buf;
    }

    GLenum e;

    MO_CHECK_GL_RET_COND(rep_, glGenBuffers(1, &buf), e);
    if (e) return invalidGl;

    MO_CHECK_GL_RET_COND(rep_, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf), e);
    if (e) goto fail;

    //MO_DEBUG("glBufferData("<<GL_ELEMENT_ARRAY_BUFFER
    //         <<", "<<(numberVertices * typeSize(valueType))
    //         <<", "<<ptr<<", "<<storageType<<")");
    MO_CHECK_GL_RET_COND(rep_, glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        numberVertices * typeSize(valueType), ptr, storageType), e);
    if (e) goto fail;

    // delete previous
    if (elementBuffer_)
    {
        MO_CHECK_GL( glDeleteBuffers(1, &elementBuffer_->id) );
    }
    else elementBuffer_ = new Buffer_;

    // keep track
    elementBuffer_->id = buf;
    elementBuffer_->target = GL_ELEMENT_ARRAY_BUFFER;
    elementBuffer_->valueType = valueType;
    elementBuffer_->numVertices = numberVertices;

    return buf;

fail:
    MO_CHECK_GL(glDeleteBuffers(1, &buf));
    return invalidGl;

}

bool VertexArrayObject::bind()
{
    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e) { return false; }

    if (elementBuffer_)
    {
        MO_CHECK_GL_RET_COND(rep_, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer_->id), e);
        if (e) { MO_CHECK_GL(glBindVertexArray(0)); return false; }
    }

    return true;
}

void VertexArrayObject::unbind()
{
    if (elementBuffer_)
        MO_CHECK_GL_COND(rep_, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL_COND(rep_, glBindVertexArray(0));
}

bool VertexArrayObject::drawElements(
        GLenum primitiveType, GLuint numberVertices, GLuint offset) const
{
    if (!elementBuffer_)
    {
        MO_GL_ERROR_COND(rep_, "no element buffer defined for VertexArrayObject::drawElements()");
        return false;
    }

    if (numberVertices <= 0)
        numberVertices = elementBuffer_->numVertices;

    GLenum e;
    MO_CHECK_GL_RET_COND(rep_, glBindVertexArray(vao_), e);
    if (e) { return false; }

    // Some drivers don't seem to store the element array in the vao state!!
    // http://stackoverflow.com/questions/8973690/vao-and-element-array-buffer-state
    MO_CHECK_GL_RET_COND(rep_, glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer_->id), e);
    if (e) { MO_CHECK_GL(glBindVertexArray(0)); return false; }

    //MO_DEBUG("glDrawElements("<<primitiveType<<", "<<numberVertices<<", "<<elementBuffer_->valueType
    //         <<", "<<reinterpret_cast<void*>(offset)<<")");
    MO_CHECK_GL_RET_COND(rep_, glDrawElements(
                                primitiveType,
                                numberVertices,
                                elementBuffer_->valueType,
                                reinterpret_cast<void*>(offset)), e);

    MO_CHECK_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    MO_CHECK_GL(glBindVertexArray(0));

    return !e;
}


} // namespace GL
} // namespace MO
