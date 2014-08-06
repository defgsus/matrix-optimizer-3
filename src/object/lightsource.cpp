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

    r_ = createFloatParameter("red", "red", tr("Red amount of light color"), 1.0, 0.1);
    g_ = createFloatParameter("green", "green", tr("Green amount of light color"), 1.0, 0.1);
    b_ = createFloatParameter("blue", "blue", tr("Blue amount of light color"), 1.0, 0.1);
}

Vec3 LightSource::lightColor(uint thread, Double time) const
{
    return Vec3(r_->value(time, thread), g_->value(time, thread), b_->value(time, thread));
}


} // namespace MO
