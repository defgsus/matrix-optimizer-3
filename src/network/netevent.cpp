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
#include "io/systeminfo.h"

namespace MO {

QMap<QString, AbstractNetEvent*> AbstractNetEvent::registeredEvents_;

std::atomic_int AbstractNetEvent::global_counter_;

AbstractNetEvent::AbstractNetEvent()
    : isValid_      (true),
      isReceived_   (false),
      isSend_       (false),
      socket_       (0),
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

    s.writeHeader("nete", 1);

    s << className();
    s << counter_;

    serialize(s);
}


bool AbstractNetEvent::send(QAbstractSocket * socket) noexcept
{
    if (socket)
        socket_ = socket;

    if (!socket_)
    {
        MO_NETLOG(ERROR, "Can't send NetEvent '" << className() << "' without socket");
        return false;
    }

    try
    {
        serialize_(*socket_);
        return isSend_ = true;
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Error sending event " << className() << "\n" << e.what());
    }
    return false;
}

AbstractNetEvent * AbstractNetEvent::receive(QAbstractSocket * s) noexcept
{
    QString cname;
    AbstractNetEvent * event = 0;

    try
    {
        QByteArray data = s->readAll();
        IO::DataStream stream(data);

        try
        {
            stream.readHeader("nete", 1);

            // get classname
            stream >> cname;

            event = createClass(cname);
            if (!event)
            {
                MO_NETLOG(ERROR, "unknown NetEvent '" << cname << "'");
                return 0;
            }

            stream >> event->counter_;

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


AbstractNetEvent * AbstractNetEvent::createResponse(const QString &name) const
{
    if (!isReceived_ || !socket_)
    {
        MO_NETLOG(WARNING, "trying to construct a NetEvent response to event '"
                  << className() << "' which was not received");
    }

    AbstractNetEvent * e = createClass(name);
    e->socket_ = socket_;
    e->counter_ = counter_;

    return e;
}

AbstractNetEvent * AbstractNetEvent::createResponseThrow(const QString &name) const
{
    auto e = createResponse(name);
    if (!e)
        MO_IO_ERROR(VERSION_MISMATCH, "Unknown NetEvent response '" << name << "'");

    return e;
}








MO_REGISTER_NETEVENT(NetEventRequest)

NetEventRequest::NetEventRequest()
    : request_    (NONE)
{
}

void NetEventRequest::serialize(IO::DataStream &io) const
{
    io << (qint64)request_ << data_;
}

void NetEventRequest::deserialize(IO::DataStream &io)
{
    qint64 r;
    io >> r >> data_;
    request_ = (Request)r;
}




MO_REGISTER_NETEVENT(NetEventInfo)

NetEventInfo::NetEventInfo()
    : request_  (NetEventRequest::NONE)
{
}

void NetEventInfo::serialize(IO::DataStream &io) const
{
    io << (qint64)request_ << data_;
}

void NetEventInfo::deserialize(IO::DataStream &io)
{
    qint64 r;
    io >> r >> data_;
    request_ = (NetEventRequest::Request)r;
}




MO_REGISTER_NETEVENT(NetEventSysInfo)

NetEventSysInfo::NetEventSysInfo()
{
}

void NetEventSysInfo::serialize(IO::DataStream &io) const
{
    info_.serialize(io);
}

void NetEventSysInfo::deserialize(IO::DataStream &io)
{
    info_.deserialize(io);
}

void NetEventSysInfo::getInfo()
{
    info_.get();
}


} // namespace MO
