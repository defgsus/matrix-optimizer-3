/** @file netevent.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#include <QIODevice>
#include <QAbstractSocket>
#include <QFile>
#include <QFileInfo>

#include "netevent.h"
#include "io/datastream.h"
#include "io/error.h"
#include "network/netlog.h"
#include "io/systeminfo.h"
#include "object/objectfactory.h"
#include "object/scene.h"

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

    if (!socket_->isWritable())
    {
        MO_NETLOG(ERROR, "Attempt to send NetEvent '" << className() << "' on unwriteable socket");
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
            event->isReceived_ = true;

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





MO_REGISTER_NETEVENT(NetEventFileInfo)

NetEventFileInfo::NetEventFileInfo()
    : present_  (false)
{
}

void NetEventFileInfo::serialize(IO::DataStream &io) const
{
    io << filename_ << time_ << present_;
}

void NetEventFileInfo::deserialize(IO::DataStream &io)
{
    io >> filename_ >> time_ >> present_;
}

void NetEventFileInfo::getFileTime()
{
    QFileInfo info(filename_);

    present_ = info.exists();
    time_ = info.lastModified();
}






MO_REGISTER_NETEVENT(NetEventFile)

NetEventFile::NetEventFile()
{
}

void NetEventFile::serialize(IO::DataStream &io) const
{
    io << filename_ << time_ << present_ << data_;
}

void NetEventFile::deserialize(IO::DataStream &io)
{
    io >> filename_ >> time_ >> present_ >> data_;
}

void NetEventFile::loadFile(const QString &fn)
{
    filename_ = fn;

    data_.clear();

    QFile f(fn);
    if (!f.open(QFile::ReadOnly))
    {
        present_ = false;
        MO_NETLOG(ERROR, "NetEventFile: failed to load file '" << fn << "'");
        return;
    }

    present_ = true;
    data_ = f.readAll();
}

bool NetEventFile::saveFile(const QString &fn) const
{
    QFile f(fn);
    if (!f.open(QFile::WriteOnly))
    {
        MO_NETLOG(ERROR, "NetEventFile: failed to write file '" << fn << "'");
        return false;
    }

    f.write(data_);
    return true;
}






MO_REGISTER_NETEVENT(NetEventScene)

NetEventScene::NetEventScene()
{
}

void NetEventScene::serialize(IO::DataStream &io) const
{
    io << data_;
}

void NetEventScene::deserialize(IO::DataStream &io)
{
    io >> data_;
}

bool NetEventScene::setScene(const Scene *scene)
{
    IO::DataStream io(&data_, QIODevice::WriteOnly);
    try
    {
        ObjectFactory::saveScene(io, scene);
        return true;
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Error on NetEventScene::setScene()\n"
                  << e.what());
    }
    return false;
}

Scene * NetEventScene::getScene() const
{
    IO::DataStream io(data_);
    try
    {
        return ObjectFactory::loadScene(io);
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Error on NetEventScene::getScene()\n"
                  << e.what());
    }
    return 0;
}

} // namespace MO
