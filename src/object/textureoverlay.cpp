/** @file textureoverlay.cpp

    @brief Texture overlay object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/12/2014</p>
*/

#include "textureoverlay.h"
#include "io/datastream.h"
#include "param/parameterfloat.h"
#include "param/parameterselect.h"
#include "gl/cameraspace.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/texture.h"
#include "gl/screenquad.h"
#include "img/image.h"

namespace MO {

MO_REGISTER_OBJECT(TextureOverlay)

TextureOverlay::TextureOverlay(QObject * parent)
    : ObjectGl      (parent)
{
    setName("TextureOverlay");
}

void TextureOverlay::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("texover", 1);
}

void TextureOverlay::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    io.readHeader("texover", 1);
}

void TextureOverlay::createParameters()
{
    ObjectGl::createParameters();

    cr_ = createFloatParameter("red", "red", tr("Red amount of color multiplier"), 1.0, 0.1);
    cr_->setMinValue(0.0);
    cg_ = createFloatParameter("green", "green", tr("Green amount of color multiplier"), 1.0, 0.1);
    cg_->setMinValue(0.0);
    cb_ = createFloatParameter("blue", "blue", tr("Blue amount of color multiplier"), 1.0, 0.1);
    cb_->setMinValue(0.0);
    ca_ = createFloatParameter("alpha", "alpha", tr("Alpha amount of color multiplier"), 1.0, 0.1);
    ca_->setMinValue(0.0);
}

void TextureOverlay::initGl(uint /*thread*/)
{
    // --- texture ---

    Image img;
    img.loadImage("/home/defgsus/prog/C/matrixoptimizer/data/graphic/kepler/bg_wood_03_polar.png");
    tex_ = GL::Texture::createFromImage(img, GL_RGBA);

    // --- shader and quad ---

    quad_ = new GL::ScreenQuad(idName());

    quad_->create(":/shader/textureoverlay.vert", ":/shader/textureoverlay.frag");

    // --- uniforms ---
    u_color_ = quad_->shader()->getUniform("u_color", true);
}

void TextureOverlay::releaseGl(uint /*thread*/)
{
    quad_->release();
    delete quad_;

    tex_->release();
    delete tex_;
}

void TextureOverlay::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    /*
    const Mat4& trans = transformation(thread, 0);
    const Mat4  cubeViewTrans = cam.cubeViewMatrix() * trans;
    const Mat4  viewTrans = cam.viewMatrix() * trans;
    */

    u_color_->setFloats(cr_->value(time, thread),
                        cg_->value(time, thread),
                        cb_->value(time, thread),
                        ca_->value(time, thread));

    tex_->bind();

    MO_CHECK_GL( glDepthMask(false) );
    quad_->draw(glContext(thread)->size().width(), glContext(thread)->size().height());
    MO_CHECK_GL( glDepthMask(true) );

}





} // namespace MO
