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

QMap<QString, AbstractNetEvent*> AbstractNetEvent::registeredEvents_;

std::atomic_int AbstractNetEvent::global_counter_;

AbstractNetEvent::AbstractNetEvent()
    : isValid_      (true),
      isReceived_   (false),
      isSend_       (false),
      counter_      (global_counter_++)
{
}

bool AbstractNetEvent::registerEventClass(AbstractNetEvent * e)
{
    MO_ASSERT(!registeredEvents_.contains(e->className()),
              "duplicate NetEvent '" << e->className() << "'");

    registeredEvents_[e->className()] = e;
    return true;
}

AbstractNetEvent * AbstractNetEvent::createClass(const QString &className)
{
    auto it = registeredEvents_.find(className);
    if (it == registeredEvents_.end())
    {
        MO_WARNING("Request for unknown NetEvent '" << className << "'");
        return 0;
    }

    return it.value()->cloneClass();
}

void AbstractNetEvent::serialize_(QIODevice &io) const
{
    IO::DataStream s(&io);

    s << className();
    s << counter_;

    serialize(s);
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

AbstractNetEvent * AbstractNetEvent::receive(QAbstractSocket * s)
{
    QString cname;
    AbstractNetEvent * event = 0;

    try
    {
        QByteArray data = s->readAll();
        IO::DataStream stream(data);

        // get classname
        stream >> cname;

        event = createClass(cname);
        if (!event)
        {
            MO_NETLOG(ERROR, "unknown NetEvent '" << cname << "'");
            return 0;
        }

        stream >> event->counter_;

        try
        {
            event->deserialize(stream);
            event->socket_ = s;
            return event;
        }
        catch (const Exception & e)
        {
            MO_NETLOG(ERROR, "error on receiving NetEvent '" << cname << "'\n"
                      << e.what());
            delete event;
            return 0;
        }
    }
    catch (...)
    {
        MO_NETLOG(ERROR, "unknown error receiving NetEvent '" << cname << "'");
        return 0;
    }
}


MO_REGISTER_NETEVENT(NetInfoEvent)

NetInfoEvent::NetInfoEvent()
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
