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

using namespace gl;

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
      drawTypeSet_      (false),
      drawType_         (GL_LINES),
      uniColor_         (0)
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::Drawable()");
}

Drawable::~Drawable()
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::~Drawable()");

    if (vao_ && vao_->isCreated())
        MO_GL_WARNING("Release of initialized vao in Drawable - OpenGL resource leak");
    delete vao_;

    delete shader_;
    delete shaderSource_;
    if (geometry_)
        geometry_->releaseRef();
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

VertexArrayObject * Drawable::vao()
{
    if (!vao_)
        vao_ = new VertexArrayObject(name_);
    return vao_;
}

bool Drawable::isReady() const
{
    return geometry_ && vao_ && vao_->isCreated() && shader_ && shader_->ready();
}

bool Drawable::isCreated() const
{
    return (vao_ && vao_->isCreated())
            || (shader_ && shader_->ready());
}

void Drawable::setGeometry(GEOM::Geometry * g)
{
    g->addRef();
    if (geometry_)
        geometry_->releaseRef();
    geometry_ = g;
    geometryChanged_ = true;
}

void Drawable::setShaderSource(ShaderSource *s)
{
    delete shaderSource_;
    shaderSource_ = s;
    doRecompile_ = true;
}

void Drawable::setShaderSource(const ShaderSource &s)
{
    if (!shaderSource_)
        shaderSource_ = new ShaderSource();
    *shaderSource_ = s;
    doRecompile_ = true;
}

void Drawable::setShader(Shader *s)
{
    delete shader_;
    shader_ = s;
    doRecompile_ = true;
}

void Drawable::setDrawType(GLenum type)
{
    drawType_ = type;
    drawTypeSet_ = true;
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

    // --- setup source ---

    if (!shaderSource_)
        shaderSource_ = new ShaderSource();

    if (shaderSource_->isEmpty())
    {
        shaderSource_->loadDefaultSource();
        doRecompile_ = true;
    }

    // --- add user attribute declarations ---

    if (geometry_->numAttributes())
    {
        QString text = "// geometry user-attributes\n";
        const QStringList list = geometry_->getAttributeNames();
        for (auto & name : list)
        {
            GEOM::Geometry::UserAttribute * attr = geometry_->getAttribute(name);
            MO_ASSERT(attr, "Declared Attribute '"
                      << name << "' not found in Geometry, Drawable '" << this->name_ << "'");
            text += attr->declaration();
        }
        shaderSource_->replace("//%user_attributes%", text);
        MO_DEBUG_GL("Drawable(" << name_ << ") added user attributes:\n" << text);
    }

    // --- create shader class ---

    if (!shader_)
    {
        shader_ = new Shader(name_);
        doRecompile_ = true;
    }

    if (!doRecompile_)
        return;

    // --- compile shader ---

    shader_->setSource(shaderSource_);
    try
    {
        if (!shader_->compile())
            MO_GL_WARNING("Compilation of Shader failed in Drawable(" << name_ << ")\n"
                          << shader_->log())
        else
            MO_DEBUG_GL("shader compiled");
    }
    catch (GlException & e)
    {
        e << "in Drawable('" << name_ << "'), shader log = {\n"
          << shader_->log() << "}";
        throw;
    }

    doRecompile_ = false;

    // --- get variable locations ---

    // XXX set default slot for normal-map
    GL::Uniform * u = shader_->getUniform("tex_norm_0");
    if (u)
        u->ints[0] = 1;

    auto diffExp = shader_->getUniform(shaderSource_->uniformNameDiffuseExponent());
    if (diffExp)
        diffExp->floats[0] = 1.0;

    uniColor_ = shader_->getUniform(shaderSource_->uniformNameColor());
    if (uniColor_)
        uniColor_->setFloats(1,1,1,1);

    if (auto u = shader_->getUniform(shaderSource_->uniformNameProjection()))
        uniformProj_ = u->location();
    else
        uniformProj_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameCubeViewTransformation()))
        uniformCVT_ = u->location();
    else
        uniformCVT_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameViewTransformation()))
        uniformVT_ = u->location();
    else
        uniformVT_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameTransformation()))
        uniformT_ = u->location();
    else
        uniformT_ = invalidGl;


    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightPos()))
        uniformLightPos_ = u->location();
    else
        uniformLightPos_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightColor()))
        uniformLightColor_ = u->location();
    else
        uniformLightColor_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightDiffuseExponent()))
        uniformLightDiffuseExp_ = u->location();
    else
        uniformLightDiffuseExp_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightDirection()))
        uniformLightDirection_ = u->location();
    else
        uniformLightDirection_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameLightDirectionMix()))
        uniformLightDirectionMix_ = u->location();
    else
        uniformLightDirectionMix_ = invalidGl;

    if (auto u = shader_->getUniform(shaderSource_->uniformNameSceneTime()))
        uniformSceneTime_ = u->location();
    else
        uniformSceneTime_ = invalidGl;
}

void Drawable::createVAO_()
{
//    MO_DEBUG_GL("Drawable(" << name_ << ")::createVAO_()");

    //MO_ASSERT(!vao_, "Drawable::createVAO_() duplicate call");

    MO_ASSERT(shader_, "Drawable(" << name_ << ")::createVAO_() without shader");

    if (!vao_)
        vao_ = new VertexArrayObject(name_);

    geometry_->getVertexArrayObject(vao_, shader_);

}

void Drawable::releaseOpenGl()
{
    MO_DEBUG_GL("Drawable(" << name_ << ")::releaseGl()");

    if (shader_)
        shader_->releaseGL();

    uniColor_ = 0;

    if (vao_)
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

    vao_->drawElements();
}


void Drawable::renderShader(const Mat4 &proj,
                            const Mat4 &cubeViewTrans,
                            const Mat4 &viewTrans,
                            const Mat4 &trans,
                            const LightSettings * lights,
                            Double time)
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable(" << name_ << ")::render()");
    //MO_ASSERT(uniformProj_ != invalidGl, "");
    //MO_ASSERT(uniformVT_ != invalidGl, "no view transformation matrix in shader");
    //MO_ASSERT(uniformT_ != invalidGl, "no transformation matrix in shader");

    checkGeometryChanged_();

    shader_->activate();

    shader_->sendUniforms();

    if (uniformProj_ != invalidGl)
        MO_CHECK_GL( glUniformMatrix4fv(uniformProj_, 1, GL_FALSE, &proj[0][0]) );

    if (uniformCVT_ != invalidGl)
        MO_CHECK_GL( glUniformMatrix4fv(uniformCVT_, 1, GL_FALSE, &cubeViewTrans[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformVT_, 1, GL_FALSE, &viewTrans[0][0]) );
    MO_CHECK_GL( glUniformMatrix4fv(uniformT_, 1, GL_FALSE, &trans[0][0]) );

    if (uniformSceneTime_ != invalidGl)
    {
        MO_CHECK_GL( glUniform1f(uniformSceneTime_, time) );
    }

    if (lights && lights->count())
    {
        if (uniformLightPos_ != invalidGl)
            MO_CHECK_GL( glUniform3fv(uniformLightPos_, lights->count(), lights->positions()) );
        if (uniformLightColor_ != invalidGl)
            MO_CHECK_GL( glUniform4fv(uniformLightColor_, lights->count(), lights->colors()) );
        if (uniformLightDirection_ != invalidGl)
            MO_CHECK_GL( glUniform4fv(uniformLightDirection_, lights->count(), lights->directions()) );
        if (uniformLightDirectionMix_ != invalidGl)
            MO_CHECK_GL( glUniform1fv(uniformLightDirectionMix_, lights->count(), lights->directionMix()) );
        if (uniformLightDiffuseExp_ != invalidGl)
            MO_CHECK_GL( glUniform1fv(uniformLightDiffuseExp_, lights->count(), lights->diffuseExponent()) );

    }

    /* XXX if (drawTypeSet_)
        vao_->drawElements(drawType_);
    else*/
        vao_->drawElements();

    shader_->deactivate();
}

void Drawable::renderShader()
{
    MO_ASSERT(vao_, "no vertex array object specified in Drawable(" << name_ << ")::render()");

    checkGeometryChanged_();

    shader_->activate();

    shader_->sendUniforms();

    /* XXX if (drawTypeSet_)
        vao_->drawElements(drawType_);
    else*/
        vao_->drawElements();

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
