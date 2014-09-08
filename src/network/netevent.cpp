/** @file netevent.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#include <QIODevice>
#include <QAbstractSocket>

#include "netevent.h"
#include "io/datastream.h"
#include "io/error.h"
#include "network/netlog.h"

namespace MO {

AbstractNetEvent::AbstractNetEvent()
    : isValid_      (true),
      isReceived_   (false),
      isSend_       (false)
{
}

void AbstractNetEvent::serialize_(QIODevice &io) const
{
    IO::DataStream s(&io);
    serialize(s);
}

void AbstractNetEvent::deserialize_(QIODevice &io)
{
    IO::DataStream s(&io);
    deserialize(s);
}

bool AbstractNetEvent::send(QAbstractSocket * socket)
{
    try
    {
        serialize_(*socket);
        return isSend_ = true;
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Error sending event " << className() << "\n" << e.what());
    }
    return false;
}


NetInfoEvent::NetInfoEvent(const QString &id)
    : id_   (id)
{

}

void NetInfoEvent::serialize(IO::DataStream &io) const
{
    io << id_ << info_;
}

void NetInfoEvent::deserialize(IO::DataStream &io)
{
    io >> id_ >> info_;
}


} // namespace MO
