/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include "oscinputobject.h"
#include "tool/linearizerfloat.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(OscInputObject)

struct OscInputObject::Private
{
    Private(OscInputObject * p)
        : p(p)
    {

    }

    OscInputObject * p;
    LinearizerFloat linear;
};


OscInputObject::OscInputObject(QObject *parent)
    : Object    (parent)
    , p_        (new Private(this))
{
    setName("OscInput");
    setNumberOutputs(ST_FLOAT, 1);
}

OscInputObject::~OscInputObject()
{
    delete p_;
}

void OscInputObject::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("osci", 1);
}

void OscInputObject::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    io.readHeader("osci", 1);
}

Double OscInputObject::valueFloat(uint , Double , uint ) const
{

}

} // namespace MO
