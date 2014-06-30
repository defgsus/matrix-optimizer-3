/** @file parameterfloat.cpp

    @brief Parameter of type float (Double)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "parameterfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(ParameterFloat)

ParameterFloat::ParameterFloat(QObject *parent)
    :   Parameter(parent),
        defaultValue_   (0.0),
        minValue_       (-10000000000),
        maxValue_       (10000000000),
        value_          (0.0)
{
    setName("Float");
}


void ParameterFloat::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("param_float", 1);

    io << defaultValue_ << minValue_ << maxValue_ << value_;
}

void ParameterFloat::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("param_float", 1);

    io >> defaultValue_ >> minValue_ >> maxValue_ >> value_;
}

Double ParameterFloat::value(Double ) const
{
    return value_;
}



} // namespace MO
