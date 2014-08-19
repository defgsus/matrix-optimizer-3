/** @file sprite.cpp

    @brief A sprite, always facing the camera

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/19/2014</p>
*/

#include "sprite.h"
#include "io/datastream.h"
#include "geom/geometryfactory.h"
#include "gl/drawable.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "param/parameterfloat.h"
//#include "param/parameterselect.h"

namespace MO {

MO_REGISTER_OBJECT(Sprite)

Sprite::Sprite(QObject * parent)
    : ObjectGl      (parent),
      draw_         (0)
{
    setName("Sprite");
}

void Sprite::serialize(IO::DataStream & io) const
{
    ObjectGl::serialize(io);

    io.writeHeader("sprite", 1);
}

void Sprite::deserialize(IO::DataStream & io)
{
    ObjectGl::deserialize(io);

    io.readHeader("sprite", 1);
}

void Sprite::createParameters()
{
    ObjectGl::createParameters();

    beginParameterGroup("color", tr("color"));

        cr_ = createFloatParameter("red", "red", tr("Red amount of ambient color"), 1.0, 0.1);
        cg_ = createFloatParameter("green", "green", tr("Green amount of ambient color"), 1.0, 0.1);
        cb_ = createFloatParameter("blue", "blue", tr("Blue amount of ambient color"), 1.0, 0.1);
        ca_ = createFloatParameter("alpha", "alpha", tr("Alpha amount of ambient color"), 1.0, 0.1);

    endParameterGroup();
}

void Sprite::onParameterChanged(Parameter *p)
{
    ObjectGl::onParameterChanged(p);
}

void Sprite::initGl(uint /*thread*/)
{
    draw_ = new GL::Drawable(idName());

    GL::ShaderSource * src = new GL::ShaderSource();

    src->loadVertexSource(":/shader/default.vert");
    src->loadFragmentSource(":/shader/default.frag");

    //src->addDefine("#define MO_ENABLE_LIGHTING");

    draw_->setShaderSource(src);

    GEOM::GeometryFactory::createQuad(draw_->geometry(), 1, 1, true);

    draw_->createOpenGl();
}

void Sprite::releaseGl(uint /*thread*/)
{
    if (draw_->isReady())
        draw_->releaseOpenGl();

    delete draw_;
    draw_ = 0;
}


void Sprite::renderGl(const GL::RenderSettings& rs, uint thread, Double time)
{
    const Mat4& trans = transformation(thread, 0);
    const Mat4  cubeViewTrans = rs.cameraSpace().cubeViewMatrix() * trans;
    const Mat4  viewTrans = rs.cameraSpace().viewMatrix() * trans;

    if (draw_->isReady())
    {
        draw_->setAmbientColor(
                    cr_->value(time, thread),
                    cg_->value(time, thread),
                    cb_->value(time, thread),
                    ca_->value(time, thread));

        draw_->renderShader(rs.cameraSpace().projectionMatrix(),
                            cubeViewTrans, viewTrans, trans, &rs.lightSettings());
    }
}





} // namespace MO
