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

    //src->addDefine("#define MO_ENABLE_BILLBOARD");

    draw_->setShaderSource(src);

    GEOM::GeometryFactory::createQuad(draw_->geometry(), 2, 2, true);

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
    const Mat4& orgtrans = transformation(thread, 0);

    // extract current position
    Vec3 pos = Vec3(-orgtrans[3][0], -orgtrans[3][1], -orgtrans[3][2]);

    // forward vector (look-at minus position)
    Vec3 f = glm::normalize(pos);
    // up vector
    Vec3 u = Vec3(orgtrans[0][1], orgtrans[1][1], orgtrans[2][1]);
    // right vector
    Vec3 s = glm::normalize(glm::cross(f, u));
    // rebuild up to avoid distortion
    u = glm::cross(s, f);

    Mat4 lookm(1);

    lookm[0][0] = s.x;
    lookm[0][1] = s.y;
    lookm[0][2] = s.z;
    lookm[1][0] = u.x;
    lookm[1][1] = u.y;
    lookm[1][2] = u.z;
    lookm[2][0] =-f.x;
    lookm[2][1] =-f.y;
    lookm[2][2] =-f.z;

    const Mat4 trans = orgtrans * lookm;

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
