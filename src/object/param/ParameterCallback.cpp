/** @file parametercallback.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#include "ParameterCallback.h"
#include "ModulatorFloat.h"
#include "object/Object.h"
#include "io/DataStream.h"

// for QMetaObject::invokeMethod
//Q_DECLARE_METATYPE(MO::ParameterCallback*);
//namespace { static int register_param = qMetaTypeId<MO::ParameterCallback*>(); }

namespace MO {

ParameterCallback::ParameterCallback(Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name)
    , p_func_       (0)
{
}

ParameterCallback::~ParameterCallback()
{

}

void ParameterCallback::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parcb", 1);

}

void ParameterCallback::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parcb", 1);
}


int ParameterCallback::getModulatorTypes() const
{
    return Object::T_TRACK_FLOAT
         | Object::T_SEQUENCE_FLOAT
         | Object::T_MODULATOR_OBJECT_FLOAT
         | Object::T_AUDIO_OBJECT;
}

void ParameterCallback::fireIfInput(const RenderTime& time)
{
    Double mod = 0;

    for (auto m : modulators())
        mod += static_cast<ModulatorFloat*>(m)->value(time);

    if (mod > 0.)
        fire();
}



Modulator * ParameterCallback::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloat(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}

} // namespace MO
