/** @file lightsource.cpp

    @brief A light source Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "LightSource.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "gl/LightSettings.h"
#include "math/vector.h"
#include "io/DataStream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(LightSource)

LightSource::LightSource()
    : Object()
{
    setName("LightSource");
}

LightSource::~LightSource() { }

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

        directionAngle_ = params()->createFloatParameter("directionangle", tr("cone angle"),
                                tr("Angle of light cone in degree"),
                                30.0,  1.);
        directionRange_ = params()->createFloatParameter("directionrange", tr("smoothness"),
                                tr("Blend area of the light cone in degree "),
                                10.0,  1.);


    params()->endParameterGroup();
}

Vec4 LightSource::lightColor(const RenderTime & time) const
{
    const Float all = all_->value(time);
    return Vec4(r_->value(time) * all,
                g_->value(time) * all,
                b_->value(time) * all,
                dist_->value(time) * 0.001);
}

void LightSource::getLightSettings(GL::LightSettings * l, uint i, const RenderTime& time)
{
    const Vec3 pos = position();
    l->setPosition(i, pos[0], pos[1], pos[2]);

    const Vec4 col = lightColor(time);
    l->setColor(i, col[0], col[1], col[2], col[3]);

    l->setDiffuseExponent(i, diffuseExp_->value(time));

    Float mix = directional_->value(time),
            rmin = 0.f, rmax = 0.f;
    if (mix > 0.f)
    {
        const Float ang = directionAngle_->value(time);
        rmax = directionRange_->value(time) / 2.;
        rmin = ang - rmax;
        rmax = ang + rmax;

        const Mat4& trans= glm::inverse(transformation());
        const Vec3 dir = MATH::normalize_safe(
                    Vec3(trans[0][2], trans[1][2], trans[2][2]));

        l->setDirection(i, dir[0], dir[1], dir[2]);
    }
    l->setDirectionParam(i, mix, rmin, rmax);
}


} // namespace MO
