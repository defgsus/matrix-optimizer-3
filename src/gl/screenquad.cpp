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


ScreenQuad::ScreenQuad(ErrorReporting reporting)
    : rep_      (reporting),
      quad_     (0)
{
}

bool ScreenQuad::create()
{
    // XXX add error reporting to Drawable and Shader

    MO_ASSERT(!quad_, "ScreenQuad::create() duplicate call");

    quad_ = new GL::Drawable();
    GEOM::GeometryFactory::createQuad(quad_->geometry(), 2, 2);
    GL::ShaderSource * src = new GL::ShaderSource();
    src->loadVertexSource(":/shader/framebufferdraw.vert");
    src->loadFragmentSource(":/shader/framebufferdraw.frag");
    quad_->setShaderSource(src);
    quad_->createOpenGl();
    return true;
}

void ScreenQuad::release()
{
    MO_ASSERT(quad_, "ScreenQuad::release() on uninitialized object");

    quad_->releaseOpenGl();
}

bool ScreenQuad::draw(uint w, uint h)
{
    Mat4 trans = h<w? glm::scale(Mat4(1.0), Vec3((float)h/w, 1, 1))
                    : glm::scale(Mat4(1.0), Vec3(1, (float)w/h, 1));
    quad_->renderShader(Mat4(1.0), trans);

    return true;
}

} // namespace GL
} // namespace MO
