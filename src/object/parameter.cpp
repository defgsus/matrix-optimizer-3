/** @file parameter.cpp

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "parameter.h"
#include "io/datastream.h"


namespace MO {

Parameter::Parameter(QObject *parent) :
    Object(parent)
{
}

void Parameter::serialize(IO::DataStream &io) const
{
    io.writeHeader("param", 1);

    io << parameterId();
}

void Parameter::deserialize(IO::DataStream &io)
{
    io.readHeader("param", 1);

    QString id;
    io >> id;
    setParameterId(id);
}

} // namespace MO
