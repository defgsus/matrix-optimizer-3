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
#include "object/trackfloat.h"
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

int ParameterFloat::getModulatorTypes() const
{
    return Object::T_TRACK_FLOAT
        | Object::T_SEQUENCE_FLOAT
        | Object::T_MODULATOR_OBJECT_FLOAT;
}

Double ParameterFloat::getModulationValue(Double time, uint thread) const
{
    Double mod = 0;

    for (auto m : modulators())
        mod += static_cast<ModulatorFloat*>(m)->value(time, thread);

    return mod;
}



Modulator * ParameterFloat::getModulator(const QString& id)
{
    Modulator * m = findModulator(id);
    if (m)
        return m;

    m = new ModulatorFloat(idName(), id, object());
    addModulator_(m);

    return m;
}



void ParameterFloat::getValues(Double time, uint thread, Double timeIncrement, uint number, Double *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time, thread);
        time += timeIncrement;
    }
}


void ParameterFloat::getValues(Double time, uint thread, Double timeIncrement, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time, thread);
        time += timeIncrement;
    }
}

void ParameterFloat::getValues(SamplePos pos, uint thread, Double sampleRateInv, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i, ++pos)
    {
        *ptr++ = value(sampleRateInv * pos, thread);
    }
}

} // namespace MO
