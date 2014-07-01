/** @file parameterfloat.cpp

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "parameterfloat.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/sequencefloat.h"
#include "object/scene.h"



Q_DECLARE_METATYPE(MO::ParameterFloat*);
namespace { static int register_param = qMetaTypeId<MO::ParameterFloat*>(); }


namespace MO {

MO_REGISTER_OBJECT(ParameterFloat)

Double ParameterFloat::infinity = 1e100;

ParameterFloat::ParameterFloat(QObject *parent)
    :   Parameter(parent),
        defaultValue_   (0.0),
        minValue_       (-infinity),
        maxValue_       (+infinity),
        value_          (0.0)
{
    setName("Float");
}


void ParameterFloat::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("param_float", 1);

    io << defaultValue_ << minValue_ << maxValue_ << value_;

    // modulations
    io << modulatorIds_;
}

void ParameterFloat::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("param_float", 1);

    io >> defaultValue_ >> minValue_ >> maxValue_ >> value_;

    io >> modulatorIds_;
}



void ParameterFloat::addModulation(const QString &idName)
{
    modulatorIds_.insert(idName);
    // hacky
    Scene * s = sceneObject();
    if (s) s->treeChanged();
}

void ParameterFloat::removeModulation(const QString &idName)
{
    modulatorIds_.remove(idName);
    // hacky
    Scene * s = sceneObject();
    if (s) s->treeChanged();
}

Double ParameterFloat::getModulation(Double time) const
{
    Double m = 0;

    for (auto s : modulators_)
        m += s->value(time);

    return m;
}

void ParameterFloat::collectModulators()
{
    modulators_.clear();

    Object * root = rootObject();

    for (auto id : modulatorIds_)
    {
        Object * o = root->findChildObject(id, true);
        if (auto s = qobject_cast<SequenceFloat*>(o))
            modulators_.append(s);
        else
            MO_WARNING("'" << idName() << "' could not find modulator '" << id << "'");
    }
}


} // namespace MO
