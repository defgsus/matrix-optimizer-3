/** @file parametercallback.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#include "parametercallback.h"
#include "io/datastream.h"

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

} // namespace MO
