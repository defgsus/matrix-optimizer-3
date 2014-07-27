/** @file drawable.cpp

    @brief An object that can be painted in OpenGL

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

/*
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
*/

#include "drawable.h"
#include "io/error.h"
#include "io/log.h"
#include "geometry.h"
#include "shadersource.h"
#include "shader.h"

namespace MO {
namespace GL {



Drawable::Drawable()
    : geometry_         (0),
      shaderSource_     (0),
      shader_           (0),
      vao_              (invalidGl),
      vertexBuffer_     (invalidGl),
      triIndexBuffer_   (invalidGl)
{
}

Drawable::~Drawable()
{
    delete shader_;
    delete shaderSource_;
    delete geometry_;
}

Geometry * Drawable::geometry()
{
    if (!geometry_)
        geometry_ = new Geometry();
    return geometry_;
}

ShaderSource * Drawable::shaderSource()
{
    if (!shaderSource_)
        shaderSource_ = new ShaderSource();
    return shaderSource_;
}

Shader * Drawable::shader()
{
    if (!shader_)
        shader_ = new Shader();
    return shader_;
}

void Drawable::setGeometry(Geometry * g)
{
    delete geometry_;
    geometry_ = g;
}

void Drawable::setShaderSource(ShaderSource *s)
{
    delete shaderSource_;
    shaderSource_ = s;
}

void Drawable::setShader(Shader *s)
{
    delete shader_;
    shader_ = s;
}


void Drawable::createOpenGl()
{
    MO_ASSERT(geometry_, "no geometry provided to Drawable::createOpenGl()");

    compileShader_();
    createVAO_();
}

void Drawable::compileShader_()
{
    MO_DEBUG_GL("Drawable::compileShader_()");

    MO_ASSERT(shaderSource_, "Drawable::compileShader_() without ShaderSource");
    MO_ASSERT(!shaderSource_->isEmpty(), "Drawable::compileShader_() with empty ShaderSource");

    if (!shader_)
        shader_ = new Shader();

    shader_->setSource(shaderSource_);
    if (!shader_->compile())
        MO_GL_WARNING("Compilation of Shader failed\n"
                      << shader_->log())
    else
        MO_DEBUG("shader compiled");

    // --- get variable locations ---

    if (auto u = shader_->getUniform(shaderSource_->uniformNameProjection()))
        uniformProj_ = u->location();
    else
        uniformProj_ = invalidGl;
    if (auto u = shader_->getUniform(shaderSource_->uniformNameView()))
        uniformView_ = u->location();
    else
        uniformView_ = invalidGl;

    if (auto a = shader_->getAttribute(shaderSource_->attribNamePosition()))
        attribPos_ = a->location();
    else
        attribPos_ = invalidGl;

}

void Drawable::createVAO_()
{
    MO_DEBUG_GL("Drawable::createVAO_()");

    MO_ASSERT(vao_==invalidGl && vertexBuffer_==invalidGl,
              "Drawable::createVAO_() duplicate call");

    MO_ASSERT(shader_, "Drawable::createVAO_() without shader");

#if (1)
    // create vertex array object
#ifdef Q_OS_APPLE
    MO_ASSERT_GL( glGenVertexArraysAPPLE(1, &vao_),
                 "Could not create vertex array object for Drawable");
#else
    MO_ASSERT_GL( glGenVertexArrays(1, &vao_),
                  "Could not create vertex array object for Drawable");
#endif

    MO_ASSERT_GL( glBindVertexArray(vao_),
                  "Could not bind vertex array object for Drawable");

    // create index buffer

    MO_ASSERT_GL( glGenBuffers(1, &triIndexBuffer_),
                  "Could not create tri index buffer for Drawable");

    MO_ASSERT_GL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBuffer_), "" );
    MO_ASSERT_GL( glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    geometry_->numTriangleIndexBytes(),
                    geometry_->triangleIndices(), GL_STATIC_DRAW), "" );
    MO_ASSERT(attribPos_ != invalidGl, "No position attribute in shader");

    // create vertex buffer

    MO_ASSERT_GL( glGenBuffers(1, &vertexBuffer_),
                  "Could not create vertex buffer for Drawable");

    MO_ASSERT_GL( glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_), "" );
    MO_ASSERT_GL( glBufferData(GL_ARRAY_BUFFER,
                    geometry_->numVertexBytes(), geometry_->vertices(), GL_STATIC_DRAW), "" );
    MO_ASSERT(attribPos_ != invalidGl, "No position attribute in shader");
    MO_ASSERT_GL( glEnableVertexAttribArray(attribPos_), "" );
    MO_ASSERT_GL( glVertexAttribPointer(attribPos_, 3, Geometry::VertexEnum, GL_FALSE, 0, NULL), "" );




    // disable bindings

    MO_CHECK_GL( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    MO_CHECK_GL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    MO_CHECK_GL( glBindVertexArray(0) );

#endif

}

void Drawable::releaseOpenGl()
{
    if (shader_)
        shader_->releaseGL();

    if (vao_ != invalidGl)
        MO_CHECK_GL( glDeleteVertexArrays(1, &vao_) );
    vao_ = invalidGl;


    /*
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
    */
}


void Drawable::render()
{
    MO_ASSERT(vao_ != invalidGl, "no vertex array object specified in Drawable::render()");

    MO_CHECK_GL( glBindVertexArray(vao_) );

    MO_CHECK_GL( glDrawElements(GL_TRIANGLES, geometry_->numTriangles() * 3,
                        Geometry::IndexEnum, geometry_->triangleIndices()) );

    MO_CHECK_GL( glBindVertexArray(0) );

    //vertexBuffer_->bind();
    //triIndexBuffer_->bind();
}


void Drawable::renderShader(const Mat4 &proj, const Mat4 &view)
{
    MO_ASSERT(vao_ != invalidGl, "no vertex array object specified in Drawable::render()");
    MO_ASSERT(uniformProj_ != invalidGl, "");
    MO_ASSERT(uniformView_ != invalidGl, "");

    shader_->activate();
    MO_CHECK_GL( glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, &proj[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformView_, 1, GL_FALSE, &view[0][0]) );

    MO_CHECK_GL( glBindVertexArray(vao_) );

    MO_ASSERT_GL( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triIndexBuffer_), "" );

/*    MO_ASSERT_GL( glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_), "" );
    MO_ASSERT_GL( glEnableVertexAttribArray(attribPos_), "" );
    MO_ASSERT_GL( glVertexAttribPointer(
                      attribPos_, 3, Geometry::VertexEnum, GL_FALSE, 0,
                      NULL), "" );
*/
    MO_DEBUG_GL( "glDrawElements" );
    MO_CHECK_GL( glDrawElements(GL_TRIANGLES, geometry_->numTriangles() * 3,
                        Geometry::IndexEnum, 0) );//geometry_->triangleIndices()) );

    MO_CHECK_GL( glBindVertexArray(0) );

    shader_->deactivate();
}

void Drawable::renderArrays()
{
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::render()");

    MO_CHECK_GL(glEnableClientState(GL_VERTEX_ARRAY) );

    MO_CHECK_GL(glVertexPointer(3, Geometry::VertexEnum, 0, geometry_->vertices()) );

    MO_CHECK_GL(glDrawElements(GL_TRIANGLES, geometry_->numTriangles() * 3,
                        Geometry::IndexEnum, geometry_->triangleIndices()) );

    MO_CHECK_GL(glDisableClientState(GL_VERTEX_ARRAY) );
}

void Drawable::renderAttribArrays()
{
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::render()");

    MO_CHECK_GL(glEnableClientState(GL_VERTEX_ARRAY) );

    MO_CHECK_GL(glVertexAttribPointer(
                    attribPos_, 3, Geometry::VertexEnum, GL_FALSE, 0, geometry_->vertices()) );

    MO_CHECK_GL(glDrawElements(GL_TRIANGLES, geometry_->numTriangles() * 3,
                        Geometry::IndexEnum, geometry_->triangleIndices()) );

    MO_CHECK_GL(glDisableClientState(GL_VERTEX_ARRAY) );
}

void Drawable::renderImmediate()
{
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::renderImmidiate()");

    glBegin(GL_TRIANGLES);
    for (uint t = 0; t<geometry_->numTriangles(); ++t)
    {
        for (int j=0; j<3; ++j)
        {
            auto v = geometry_->triangle(t, j);
            glVertex3f(v[0], v[1], v[2]);
        }
    }
    MO_CHECK_GL( glEnd() );
}

void Drawable::renderImmediateShader(const Mat4& proj, const Mat4& view)
{
    MO_ASSERT(geometry_, "no Geometry specified in Drawable::renderImmidiate()");
    MO_ASSERT(uniformProj_ != invalidGl, "");
    MO_ASSERT(uniformView_ != invalidGl, "");
    MO_ASSERT(attribPos_ != invalidGl, "");

    shader_->activate();
    MO_CHECK_GL( glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, &proj[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformView_, 1, GL_FALSE, &view[0][0]) );

    glBegin(GL_TRIANGLES);
    for (uint t = 0; t<geometry_->numTriangles(); ++t)
    {
        for (int j=0; j<3; ++j)
        {
            auto v = geometry_->triangle(t, j);
            glVertexAttrib3f(attribPos_, v[0], v[1], v[2]);
        }
    }
    MO_CHECK_GL( glEnd() );

    shader_->deactivate();
}

} //namespace GL
} // namespace MO
