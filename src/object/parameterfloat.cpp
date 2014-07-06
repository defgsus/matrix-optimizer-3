/** @file parameterfloat.cpp

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "parameterfloat.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/trackfloat.h"
#include "object/scene.h"
#include "io/datastream.h"




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

    io.writeHeader("parf", 1);

    io << defaultValue_ << minValue_ << maxValue_ << value_;

}

void ParameterFloat::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parf", 1);

    io >> defaultValue_ >> minValue_ >> maxValue_ >> value_;
}


Double ParameterFloat::getModulationValue(Double time) const
{
    Double m = 0;

    for (auto t : modulators_)
        m += t->value(time);

    return m;
}

void ParameterFloat::collectModulators()
{
    modulators_.clear();

    Object * root = rootObject();

    for (auto const &id : getModulators())
    {
        Object * o = root->findChildObject(id, true);
        if (auto s = qobject_cast<TrackFloat*>(o))
            modulators_.append(s);
        else
            MO_WARNING("parameter '" << idName()
                       << "' could not find modulator '" << id << "'");
    }
}


} // namespace MO
