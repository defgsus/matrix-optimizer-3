/** @file lightsource.cpp

    @brief A light source Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "lightsource.h"
#include "io/datastream.h"
#include "param/parameters.h"
#include "param/parameterfloat.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(LightSource)

LightSource::LightSource(QObject *parent) :
    Object(parent)
{
    setName("LightSource");
}

void LightSource::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("light", 1);
}

void LightSource::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("light", 1);
}

void LightSource::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("color", tr("color"));
    initParameterGroupExpanded("color");

        all_ = params()->createFloatParameter("all", tr("brightness"), tr("Amplifier for all colors"), 1.0, 0.1);
        r_ = params()->createFloatParameter("red", tr("red"), tr("Red amount of light color"), 1.0, 0.1);
        g_ = params()->createFloatParameter("green", tr("green"), tr("Green amount of light color"), 1.0, 0.1);
        b_ = params()->createFloatParameter("blue", tr("blue"), tr("Blue amount of light color"), 1.0, 0.1);

    params()->endParameterGroup();

    params()->beginParameterGroup("light", tr("light settings"));

        dist_ = params()->createFloatParameter("dist", tr("distance attenuation"),
                                     tr("Distance attenuation factor - "
                                        "the higher, the less light reaches distant places"), 0, 0.1);
        dist_->setMinValue(0.0);

        diffuseExp_ = params()->createFloatParameter("diffuseexp", tr("diffuse exponent"),
                                tr("Multiplier for diffuse lighting exponent"
                                   "- the higher, the less light reflects off surfaces not facing the lightsource"),
                                1.0, 0.1);
        diffuseExp_->setMinValue(0.001);

        directional_ = params()->createFloatParameter("directional", tr("directional"),
                                tr("Mix between omni-directional (0) and directional light (1) "),
                                0.0, 0.0, 1.0, 0.1);
        directionExp_ = params()->createFloatParameter("directionexp", tr("angle exponent"),
                                tr("Exponent for the directional influence "
                                   "- the higher, the narrower the angle"),
                                10.0, 0.1);
        directionExp_->setMinValue(0.001);

    params()->endParameterGroup();
}

Vec4 LightSource::lightColor(uint thread, Double time) const
{
    const Float all = all_->value(time, thread);
    return Vec4(r_->value(time, thread) * all,
                g_->value(time, thread) * all,
                b_->value(time, thread) * all,
                dist_->value(time, thread) * 0.001);
}

Vec4 LightSource::lightDirection(uint thread, Double time) const
{
    const Mat4& trans= glm::inverse(transformation());
    const Vec3 dir = MATH::normalize_safe(
                Vec3(trans[0][2], trans[1][2], trans[2][2]));
    return Vec4(dir[0], dir[1], dir[2], directionExp_->value(time, thread));
}

Float LightSource::lightDirectionalMix(uint thread, Double time) const
{
    return directional_->value(time, thread);
}

Float LightSource::diffuseExponent(uint thread, Double time) const
{
    return diffuseExp_->value(time, thread);
}

} // namespace MO
