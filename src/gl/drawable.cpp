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
#include "vertexarrayobject.h"

namespace MO {
namespace GL {



Drawable::Drawable()
    : geometry_         (0),
      shaderSource_     (0),
      shader_           (0),
      vao_              (0)
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
    if (auto a = shader_->getAttribute(shaderSource_->attribNameColor()))
        attribColor_ = a->location();
    else
        attribColor_ = invalidGl;

}

void Drawable::createVAO_()
{
    MO_DEBUG_GL("Drawable::createVAO_()");

    MO_ASSERT(!vao_, "Drawable::createVAO_() duplicate call");

    MO_ASSERT(shader_, "Drawable::createVAO_() without shader");
    MO_ASSERT(attribPos_ != invalidGl, "No position attribute in shader");
    MO_ASSERT(attribColor_ != invalidGl, "No color attribute in shader");

    vao_ = new VertexArrayObject(ER_THROW);

    vao_->create();
    vao_->bind();

    if (geometry_->numTriangles())
        vao_->createIndexBuffer(geometry_->IndexEnum,
                                geometry_->numTriangles() * 3,
                                geometry_->triangleIndices());
    else
        vao_->createIndexBuffer(geometry_->IndexEnum,
                                geometry_->numLines() * 2,
                                geometry_->lineIndices());

    vao_->createAttribBuffer(attribPos_,
                             geometry_->VertexEnum, 3,
                             geometry_->numVertexBytes(),
                             geometry_->vertices());
    vao_->createAttribBuffer(attribColor_,
                             geometry_->ColorEnum, 4,
                             geometry_->numColorBytes(),
                             geometry_->colors());

    vao_->unbind();


}

void Drawable::releaseOpenGl()
{
    if (shader_)
        shader_->releaseGL();

    if (!vao_)
    {
        vao_->release();
        delete vao_;
    }
    vao_ = 0;
}


void Drawable::render()
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable::render()");

    if (geometry_->numTriangles())
        vao_->drawElements(GL_TRIANGLES);
    else
        vao_->drawElements(GL_LINES);
}


void Drawable::renderShader(const Mat4 &proj, const Mat4 &view)
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable::render()");
    MO_ASSERT(uniformProj_ != invalidGl, "");
    MO_ASSERT(uniformView_ != invalidGl, "");

    shader_->activate();
    MO_CHECK_GL( glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, &proj[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformView_, 1, GL_FALSE, &view[0][0]) );

    if (geometry_->numTriangles())
        vao_->drawElements(GL_TRIANGLES);
    else
        vao_->drawElements(GL_LINES);

    shader_->deactivate();
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
