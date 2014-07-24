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


Double ParameterFloat::getModulationValue(Double time) const
{
    Double m = 0;

    for (auto t : modulators_)
        if (t->active(time))
            m += t->value(time);

    return m;
}

void ParameterFloat::collectModulators()
{
    modulators_.clear();
    if (modulatorIds().isEmpty())
        return;

    MO_DEBUG_MOD("ParameterFloat("<<idName()<<")::collectModulators()");


    Object * root = object()->rootObject();

    for (auto const &id : modulatorIds())
    {
        Object * o = root->findChildObject(id, true);
        if (auto s = qobject_cast<TrackFloat*>(o))
            modulators_.append(s);
        else
            MO_WARNING("parameter '" << idName()
                       << "' could not find modulator '" << id << "'");
    }

    MO_DEBUG_MOD("ParameterFloat("<<idName()<<") found " << modulators_.size() << " modulator(s)");
}

QList<Object*> ParameterFloat::getModulatingObjects() const
{
    QList<Object*> list;

    for (auto m : modulators_)
        list.append(m);

    for (auto m : modulators_)
        list.append(m->getModulatingObjects());

    return list;
}

QList<Object*> ParameterFloat::getFutureModulatingObjects(const Scene *scene) const
{
    QList<Object*> mods, list;

    for (const auto &m : modulatorIds())
    {
        if (Object * o = scene->findChildObject(m, true))
            mods.append(o);
    }

    list = mods;

    for (auto m : mods)
        list.append(m->getModulatingObjects());

    return list;
}


void ParameterFloat::getValues(Double time, Double timeIncrement, uint number, Double *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time);
        time += timeIncrement;
    }
}


void ParameterFloat::getValues(Double time, Double timeIncrement, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i)
    {
        *ptr++ = value(time);
        time += timeIncrement;
    }
}

void ParameterFloat::getValues(SamplePos pos, Double sampleRateInv, uint number, F32 *ptr) const
{
    for (uint i=0; i<number; ++i, ++pos)
    {
        *ptr++ = value(sampleRateInv * pos);
    }
}

} // namespace MO
