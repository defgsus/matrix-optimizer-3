/** @file drawable.cpp

    @brief An object that can be painted in OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "drawable.h"
#include "io/error.h"
#include "io/log.h"
#include "geometry.h"

namespace MO {
namespace GL {



Drawable::Drawable(MO_QOPENGL_FUNCTIONS_CLASS * functions)
    : gl_               (functions),
      geometry_         (0),
      vertexBuffer_     (0),
      triIndexBuffer_   (0)
{
}

Drawable::~Drawable()
{
    delete vertexBuffer_;
    delete triIndexBuffer_;
}

Geometry * Drawable::geometry()
{
    if (!geometry_)
        geometry_ = new Geometry();
    return geometry_;
}

void Drawable::setGeometry(Geometry * g)
{
    delete geometry_;
    geometry_ = g;
}


void Drawable::createOpenGl()
{
    MO_ASSERT(geometry_, "no geometry provided to Drawable::createOpenGl()");

    MO_ASSERT(!vertexBuffer_, "Drawable::createOpenGl() duplicate call");

    // create vertex buffer

    vertexBuffer_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);

    if (!vertexBuffer_->create())
        MO_GL_ERROR("Could not create vertex buffer for Drawable");
    vertexBuffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!vertexBuffer_->bind())
        MO_GL_ERROR("Could not bind vertex buffer for Drawable");

    vertexBuffer_->allocate(geometry_->vertices(), geometry_->numVertexBytes());

    // create triangle index buffer

    triIndexBuffer_ = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    if (!triIndexBuffer_->create())
        MO_GL_ERROR("Could not create triangle index buffer for Drawable");
    triIndexBuffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
    if (!triIndexBuffer_->bind())
        MO_GL_ERROR("Could not bind vertex buffer for Drawable");

    triIndexBuffer_->allocate(geometry_->triangleIndices(), geometry_->numTriangleIndexBytes());

}

void Drawable::releaseOpenGl()
{
    if (vertexBuffer_)
    {
        vertexBuffer_->release();
        delete vertexBuffer_;
        vertexBuffer_ = 0;
    }

    if (triIndexBuffer_)
    {
        triIndexBuffer_->release();
        delete triIndexBuffer_;
        triIndexBuffer_ = 0;
    }
}


void Drawable::render()
{
    MO_ASSERT(gl_, "Drawable::render() without QOpenGLFunctions");
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::render()");

    /*
    MO_DEBUG("Drawable::draw()\n"
             << "vertices: " << geometry_->numVertices()
             << "\ntriangles: " << geometry_->numTriangles()
             );
    */

    //vertexBuffer_->bind();
    //triIndexBuffer_->bind();
    MO_CHECK_GL(gl_, glEnableClientState(GL_VERTEX_ARRAY) );

    MO_CHECK_GL(gl_, glVertexPointer(3, Geometry::VertexEnum, 0, geometry_->vertices()) );

    MO_CHECK_GL(gl_, glDrawElements(GL_TRIANGLES, geometry_->numTriangles() * 3,
                        Geometry::IndexEnum, geometry_->triangleIndices()) );

    MO_CHECK_GL(gl_, glDisableClientState(GL_VERTEX_ARRAY) );
}

void Drawable::renderImmidiate()
{
    MO_ASSERT(gl_, "Drawable::renderImmidiate() without QOpenGLFunctions");
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::renderImmidiate()");

    MO_CHECK_GL(gl_, glBegin(GL_TRIANGLES) );
    for (uint t = 0; t<geometry_->numTriangles(); ++t)
    {
        for (int j=0; j<3; ++j)
        {
            auto v = geometry_->triangle(t, j);
            MO_CHECK_GL(gl_, glVertex3f(v[0], v[1], v[2]) );
        }
    }
    MO_CHECK_GL(gl_, glEnd() );
}

} //namespace GL
} // namespace MO
