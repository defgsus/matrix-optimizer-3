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
#include "geom/geometry.h"
#include "shadersource.h"
#include "shader.h"
#include "vertexarrayobject.h"
#include "lightsettings.h"

namespace MO {
namespace GL {



Drawable::Drawable(const QString &name)
    : name_             (name.isEmpty()? "unnamed" : name),
      geometry_         (0),
      shaderSource_     (0),
      shader_           (0),
      doRecompile_      (true),
      geometryChanged_  (false),
      vao_              (0),
      uniColor_         (0)
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::Drawable()");
}

Drawable::~Drawable()
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::~Drawable()");

    delete shader_;
    delete shaderSource_;
    delete geometry_;
}

GEOM::Geometry * Drawable::geometry()
{
    if (!geometry_)
        geometry_ = new GEOM::Geometry();
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
        shader_ = new Shader(name_);
    return shader_;
}

bool Drawable::isReady() const
{
    return geometry_ && vao_ && vao_->isCreated() && shader_ && shader_->ready();
}

void Drawable::setGeometry(GEOM::Geometry * g)
{
    delete geometry_;
    geometry_ = g;
    geometryChanged_ = true;
}

void Drawable::setShaderSource(ShaderSource *s)
{
    delete shaderSource_;
    shaderSource_ = s;
    doRecompile_ = true;
}

void Drawable::setShader(Shader *s)
{
    delete shader_;
    shader_ = s;
    doRecompile_ = true;
}


void Drawable::createOpenGl()
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::createOpenGl()");

    MO_ASSERT(geometry_, "no geometry provided to Drawable(" << name_ << ")::createOpenGl()");

    compileShader_();
    createVAO_();
}

void Drawable::compileShader_()
{
//    MO_DEBUG_GL("Drawable(" << name_ << ")::compileShader_()");

    //MO_ASSERT(shaderSource_, "Drawable::compileShader_() without ShaderSource");
    //MO_ASSERT(!shaderSource_->isEmpty(), "Drawable::compileShader_() with empty ShaderSource");

    if (!shaderSource_)
        shaderSource_ = new ShaderSource();

    if (shaderSource_->isEmpty())
    {
        shaderSource_->loadDefaultSource();
        doRecompile_ = true;
    }

    if (!shader_)
    {
        shader_ = new Shader(name_);
        doRecompile_ = true;
    }

    if (!doRecompile_)
        return;

    shader_->setSource(shaderSource_);
    if (!shader_->compile())
        MO_GL_WARNING("Compilation of Shader failed in Drawable(" << name_ << ")\n"
                      << shader_->log())
    else
        MO_DEBUG("shader compiled");

    doRecompile_ = false;

    // --- get variable locations ---

    uniColor_ = shader_->getUniform(shaderSource_->uniformNameColor());
    if (uniColor_)
        uniColor_->setFloats(1,1,1,1);

    if (auto u = shader_->getUniform(shaderSource_->uniformNameProjection()))
        uniformProj_ = u->location();
    else
        uniformProj_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameView()))
        uniformView_ = u->location();
    else
        uniformView_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameTransformation()))
        uniformTransform_ = u->location();
    else
        uniformTransform_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightPos()))
        uniformLightPos_ = u->location();
    else
        uniformLightPos_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightColor()))
        uniformLightColor_ = u->location();
    else
        uniformLightColor_ = invalidGl;

}

void Drawable::createVAO_()
{
//    MO_DEBUG_GL("Drawable(" << name_ << ")::createVAO_()");

    //MO_ASSERT(!vao_, "Drawable::createVAO_() duplicate call");

    MO_ASSERT(shader_, "Drawable(" << name_ << ")::createVAO_() without shader");

    if (!vao_)
        vao_ = new VertexArrayObject(ER_THROW);

    geometry_->getVertexArrayObject(vao_, shader_, geometry_->numTriangles() != 0);

}

void Drawable::releaseOpenGl()
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::releaseGl()");

    if (shader_)
        shader_->releaseGL();

    uniColor_ = 0;

    if (!vao_)
    {
        vao_->release();
        delete vao_;
    }
    vao_ = 0;
}

void Drawable::setAmbientColor(Float r, Float g, Float b, Float a)
{
    if (uniColor_)
        uniColor_->setFloats(r, g, b, a);
}

void Drawable::render()
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable(" << name_ << ")::render()");

    checkGeometryChanged_();

    if (geometry_->numTriangles())
        vao_->drawElements(GL_TRIANGLES);
    else
        vao_->drawElements(GL_LINES);
}


void Drawable::renderShader(const Mat4 &proj, const Mat4 &view,
                            const Mat4 * orgView, const LightSettings * lights)
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable(" << name_ << ")::render()");
    //MO_ASSERT(uniformProj_ != invalidGl, "");
    MO_ASSERT(uniformView_ != invalidGl, "");

    checkGeometryChanged_();

    shader_->activate();

    shader_->sendUniforms();

    if (uniformProj_ != invalidGl)
        MO_CHECK_GL( glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, &proj[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformView_, 1, GL_FALSE, &view[0][0]) );

    if (uniformTransform_ != invalidGl)
    {
        if (orgView)
            MO_CHECK_GL( glUniformMatrix4fv(uniformTransform_, 1, GL_FALSE, &(*orgView)[0][0]) )
        else
            MO_CHECK_GL( glUniformMatrix4fv(uniformTransform_, 1, GL_FALSE, &view[0][0]) );
    }

    if (lights && lights->count())
    {
        if (uniformLightPos_ != invalidGl)
            MO_CHECK_GL( glUniform3fv(uniformLightPos_, lights->count(), lights->positions()) );
        if (uniformLightColor_ != invalidGl)
            MO_CHECK_GL( glUniform3fv(uniformLightColor_, lights->count(), lights->colors()) );
    }

    if (geometry_->numTriangles())
        vao_->drawElements(GL_TRIANGLES);
    else
        vao_->drawElements(GL_LINES);

    shader_->deactivate();
}

void Drawable::renderShader()
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable(" << name_ << ")::render()");

    checkGeometryChanged_();

    shader_->activate();

    shader_->sendUniforms();

    if (geometry_->numTriangles())
        vao_->drawElements(GL_TRIANGLES);
    else
        vao_->drawElements(GL_LINES);

    shader_->deactivate();
}


void Drawable::renderImmediate()
{
    MO_ASSERT(geometry_, "no Geometry specified in Drawable(" << name_ << ")::renderImmidiate()");

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

void Drawable::checkGeometryChanged_()
{
    if (geometryChanged_)
    {
        createOpenGl();
        geometryChanged_ = false;
    }
}

} //namespace GL
} // namespace MO
