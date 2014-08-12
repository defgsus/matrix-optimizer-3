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
    cg_ = createFloatParameter("green", "green", tr("Green amount of color multiplier"), 1.0, 0.1);
    cb_ = createFloatParameter("blue", "blue", tr("Blue amount of color multiplier"), 1.0, 0.1);
    ca_ = createFloatParameter("alpha", "alpha", tr("Alpha amount of color multiplier"), 1.0, 0.1);
}

void TextureOverlay::initGl(uint /*thread*/)
{
    Image img;
    img.loadImage("/home/defgsus/prog/C/matrixoptimizer/data/graphic/kepler/bg_wood_03_polar.png");
}

void TextureOverlay::releaseGl(uint /*thread*/)
{
}

void TextureOverlay::renderGl(const GL::CameraSpace& cam, uint thread, Double time)
{
    const Mat4& trans = transformation(thread, 0);
    const Mat4  cubeViewTrans = cam.cubeViewMatrix() * trans;
    const Mat4  viewTrans = cam.viewMatrix() * trans;
}





} // namespace MO
