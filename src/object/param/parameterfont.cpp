/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/22/2015</p>
*/

#include "parameterfont.h"
#include "object/object.h"
#include "io/datastream.h"

namespace MO {

ParameterFont::ParameterFont(Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name)
{
}

ParameterFont::~ParameterFont()
{

}

void ParameterFont::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parfo", 1);

    io << p_font_.toString();

}

void ParameterFont::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parfo", 1);

    QString s;
    io >> s;
    p_font_.fromString(s);
}


void ParameterFont::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterFont*>(other);
    if (!p)
        return;
    p_font_ = p->p_font_;
}


} // namespace MO
