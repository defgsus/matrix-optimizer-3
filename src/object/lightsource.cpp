/** @file light.cpp

    @brief A light source

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "lightsource.h"
#include "io/datastream.h"
#include "param/parameterfloat.h"


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

    r_ = createFloatParameter("red", tr("red"), tr("Red amount of light color"), 1.0, 0.1);
    g_ = createFloatParameter("green", tr("green"), tr("Green amount of light color"), 1.0, 0.1);
    b_ = createFloatParameter("blue", tr("blue"), tr("Blue amount of light color"), 1.0, 0.1);
    dist_ = createFloatParameter("dist", tr("distance attenuation"),
                                 tr("Distance attenuation factor - "
                                    "the higher the less light reaches distant places"), 0, 0.1);
    dist_->setMinValue(0.0);
}

Vec4 LightSource::lightColor(uint thread, Double time) const
{
    return Vec4(r_->value(time, thread),
                g_->value(time, thread),
                b_->value(time, thread),
                dist_->value(time, thread));
}


} // namespace MO
