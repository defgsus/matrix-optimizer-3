/** @file screenquad.cpp

    @brief Screen-sized texture quad painter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/1/2014</p>
*/

#include "screenquad.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/drawable.h"
#include "geom/geometryfactory.h"

namespace MO {
namespace GL {


ScreenQuad::ScreenQuad(const QString &name, ErrorReporting reporting)
    : name_         (name.isEmpty()? "quad" : name)
    , rep_          (reporting)
    , quad_         (0)
    , antialias_    (1)
    , u_resolution_ (0)
{
}

ScreenQuad::~ScreenQuad()
{
    if (quad_ && quad_->isReady())
        MO_GL_WARNING("Release of initialized ScreenQuad - OpenGL resource leak");
    delete quad_;
}

bool ScreenQuad::isCreated() const
{
    return (quad_ && quad_->isCreated());
}

void ScreenQuad::setAntialiasing(uint samples)
{
    antialias_ = samples;
}

Shader * ScreenQuad::shader() const
{
    return quad_ ? quad_->shader() : 0;
}

bool ScreenQuad::create(const QString &vertexFile, const QString &fragmentFile,
                        const QString& defines,
                        GEOM::Geometry * geom)
{
    /// @todo add error reporting to Drawable and Shader

    MO_ASSERT(!quad_, "ScreenQuad::create() duplicate call");

    // prepare geometry

    quad_ = new GL::Drawable(name_);
    if (!geom)
        GEOM::GeometryFactory::createQuad(quad_->geometry(), 2, 2);
    else
        quad_->setGeometry(geom);

    // prepare source

    GL::ShaderSource * src = new GL::ShaderSource();
    src->loadVertexSource(vertexFile);
    src->loadFragmentSource(fragmentFile);

    if (!defines.isEmpty())
        src->addDefine(defines);

    if (antialias_ > 1)
        src->addDefine(QString("#define MO_ANTIALIAS (%1)").arg(antialias_));

    quad_->setShaderSource(src);
    quad_->createOpenGl();

    if (antialias_ > 1)
        u_resolution_ = quad_->shader()->getUniform("u_resolution", true);

    return true;
}

bool ScreenQuad::create(ShaderSource * src, GEOM::Geometry * geom)
{
    MO_ASSERT(!quad_, "ScreenQuad::create() duplicate call");

    // prepare geometry

    quad_ = new GL::Drawable(name_);
    if (!geom)
        GEOM::GeometryFactory::createQuad(quad_->geometry(), 2, 2.);
    else
        quad_->setGeometry(geom);

    quad_->setShaderSource(src);
    quad_->createOpenGl();

    return true;
}


void ScreenQuad::release()
{
    MO_ASSERT(quad_, "ScreenQuad::release() on uninitialized object");

    quad_->releaseOpenGl();

    delete quad_;
    quad_ = 0;
}


bool ScreenQuad::draw(uint w, uint h, uint splits)
{
    if (u_resolution_)
        u_resolution_->setFloats(w, h, (Float)1 / w, (Float)1 / h);

    // simply stretch the quad into view
    Mat4 trans = Mat4(1.0);

    if (splits <= 1)
    {
        Mat4 trans = Mat4(1.0);
        quad_->renderShader(trans, trans, trans, trans);
    }
    else

    // render individual slices
    for (uint i=0; i<splits; ++i)
    {
        trans[1].y = 1.f / splits;
        trans[3].y = Float(i + .5) / splits * 2.f - 1.f;
        quad_->renderShader(trans, trans, trans, trans);
        gl::glFlush();
    }

    return true;
}

bool ScreenQuad::drawCentered(uint w, uint h)
{
    if (u_resolution_)
        u_resolution_->setFloats(w, h, (Float)1 / w, (Float)1 / h);

    // center the quad into view
    Mat4 trans = h<w? glm::scale(Mat4(1.0), Vec3((Float)h/w, 1, 1))
                    : glm::scale(Mat4(1.0), Vec3(1, (Float)w/h, 1));
    quad_->renderShader(Mat4(1.0), trans, trans, trans);

    return true;
}


bool ScreenQuad::drawCentered(uint iw, uint ih, Float aspect)
{
    Float w = iw, h = ih * aspect;

    if (u_resolution_)
        u_resolution_->setFloats(w, h, (Float)1 / w, (Float)1 / h);

    // center the quad into view
    Mat4 trans = h<w? glm::scale(Mat4(1.0), Vec3((Float)h/w, 1,          1))
                    : glm::scale(Mat4(1.0), Vec3(         1, (Float)w/h, 1));
    quad_->renderShader(Mat4(1.0), trans, trans, trans);

    return true;
}

} // namespace GL
} // namespace MO
