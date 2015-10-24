/** @file parameterfloat.cpp

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/


#include "parameterfloat.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "modulatorfloat.h"

// make ParameterFloat useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterFloat*);
namespace { static int register_param = qMetaTypeId<MO::ParameterFloat*>(); }


namespace MO {

Double ParameterFloat::infinity = 1e100;

ParameterFloat::ParameterFloat(Object * object, const QString& id, const QString& name)
    :   Parameter(object, id, name),
        defaultValue_   (0.0),
        minValue_       (-infinity),
        maxValue_       (+infinity),
        smallStep_      (1.0),
        value_          (0.0)
{
}


void ParameterFloat::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parf", 1);

    io << value_;

}

void ParameterFloat::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parf", 1);

    io >> value_;
}

QString ParameterFloat::getDocType() const
{
    QString str = typeName();

    // get range string
    bool limmin = minValue() > -infinity,
         limmax = maxValue() < infinity;
    if (limmin || limmax)
    {
        if (limmin && limmax)
            str += " " + QObject::tr("range [%1, %2]").arg(minValue()).arg(maxValue());
        else if (limmin)
            str += " >= " + QString::number(minValue());
        else
            str += " <= " + QString::number(maxValue());
    }

    return str;
}

int ParameterFloat::getModulatorTypes() const
{
    return Object::T_TRACK_FLOAT
         | Object::T_SEQUENCE_FLOAT
         | Object::T_MODULATOR_OBJECT_FLOAT
         | Object::T_AUDIO_OBJECT;
}

Double ParameterFloat::getModulationValue(const RenderTime& time) const
{
    Double mod = 0;

    for (auto m : modulators())
        mod += static_cast<ModulatorFloat*>(m)->value(time);

    return mod;
}



Modulator * ParameterFloat::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloat(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}



void ParameterFloat::getValues(const RenderTime & time, Double timeIncrement, uint number, Double *ptr) const
{
    RenderTime rtime(time);
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(rtime);
        rtime.setSecond( rtime.second() + timeIncrement );
    }
}


void ParameterFloat::getValues(const RenderTime & time, Double timeIncrement, uint number, F32 *ptr) const
{
    RenderTime rtime(time);
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(rtime);
        rtime.setSecond( rtime.second() + timeIncrement );
    }
}
/*
void ParameterFloat::getValues(const RenderTime & time, Double sampleRateInv, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i, ++time)
    {
        *ptr++ = value(sampleRateInv * time, thread);
    }
}*/

} // namespace MO
