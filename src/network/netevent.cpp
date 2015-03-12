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
#include "tool/stringmanip.h"

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

QString AbstractNetEvent::infoName() const
{
    return QString("%1[%2]").arg(className()).arg(counter());
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

QString NetEventRequest::infoName() const
{
    return AbstractNetEvent::infoName() + ":" + requestName(request_);
}

QString NetEventRequest::requestName(Request r)
{
    switch (r)
    {
        case NONE: return "NONE";
        case GET_SYSTEM_INFO: return "GET_SYSTEM_INFO";
        case GET_CLIENT_STATE: return "GET_CLIENT_STATE";
        case SET_CLIENT_INDEX: return "SET_CLIENT_INDEX";
        case SET_DESKTOP_INDEX: return "SET_DESKTOP_INDEX";
        case SHOW_INFO_WINDOW: return "SHOW_INFO_WINDOW";
        case HIDE_INFO_WINDOW: return "HIDE_INFO_WINDOW";
        case SHOW_RENDER_WINDOW: return "SHOW_RENDER_WINDOW";
        case HIDE_RENDER_WINDOW: return "HIDE_RENDER_WINDOW";
        case START_RENDER: return "START_RENDER";
        case STOP_RENDER: return "STOP_RENDER";
        case SET_PROJECTION_SETTINGS: return "SET_PROJECTION_SETTINGS";
        case GET_SERVER_FILE_TIME: return "GET_SERVER_FILE_TIME";
        case GET_SERVER_FILE: return "GET_SERVER_FILE";
    }
    return "*UNKNOWN*";
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





MO_REGISTER_NETEVENT(NetEventLog)

NetEventLog::NetEventLog()
{
}

QString NetEventLog::infoName() const
{
    return AbstractNetEvent::infoName();
}

void NetEventLog::serialize(IO::DataStream &io) const
{
    io << (qint64)level_ << message_;
}

void NetEventLog::deserialize(IO::DataStream &io)
{
    qint64 l;
    io >> l >> message_;
    level_ = (NetworkLogger::Level)l;
}






MO_REGISTER_NETEVENT(NetEventInfo)

NetEventInfo::NetEventInfo()
    : request_  (NetEventRequest::NONE)
{
}

QString NetEventInfo::infoName() const
{
    return QString("%1:%2(%3)")
            .arg(AbstractNetEvent::infoName())
            .arg(NetEventRequest::requestName(request_))
            .arg(data_.typeName());
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

QString NetEventSysInfo::infoName() const
{
    return AbstractNetEvent::infoName();
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

QString NetEventFileInfo::infoName() const
{
    return AbstractNetEvent::infoName() + "(.." + filename_.right(18) + ")";
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

QString NetEventFile::infoName() const
{
    return AbstractNetEvent::infoName() + "(.." + filename_.right(18) + ")";
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
    time_ = QFileInfo(fn).lastModified();
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

QString NetEventScene::infoName() const
{
    return AbstractNetEvent::infoName() + "(" + byte_to_string(data_.size()) + ")";
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








MO_REGISTER_NETEVENT(NetEventClientState)

NetEventClientState::NetEventClientState()
{
}

QString NetEventClientState::infoName() const
{
    return AbstractNetEvent::infoName();
}

void NetEventClientState::serialize(IO::DataStream &io) const
{
    state_.serialize(io);
}

void NetEventClientState::deserialize(IO::DataStream &io)
{
    state_.deserialize(io);
}





MO_REGISTER_NETEVENT(NetEventAudioConfig)

NetEventAudioConfig::NetEventAudioConfig()
{
}

QString NetEventAudioConfig::infoName() const
{
    std::stringstream s; s << config_;
    return AbstractNetEvent::infoName() + QString("[%1]").arg(s.str().c_str());
}

void NetEventAudioConfig::serialize(IO::DataStream &io) const
{
    io << config_.sampleRate() << config_.bufferSize()
       << config_.numChannelsIn() << config_.numChannelsOut();
}

void NetEventAudioConfig::deserialize(IO::DataStream &io)
{
    uint sr, bs, nin, nout;
    io >> sr >> bs >> nin >> nout;
    config_ = AUDIO::Configuration(sr, bs, nin, nout);
}




MO_REGISTER_NETEVENT(NetEventTime)

NetEventTime::NetEventTime()
{
}

QString NetEventTime::infoName() const
{
    return AbstractNetEvent::infoName() + "(" + QString::number(time_) + ")";
}

void NetEventTime::serialize(IO::DataStream &io) const
{
    io << time_;
}

void NetEventTime::deserialize(IO::DataStream &io)
{
    io >> time_;
}






MO_REGISTER_NETEVENT(NetEventUiFloat)

NetEventUiFloat::NetEventUiFloat()
{
}

QString NetEventUiFloat::infoName() const
{
    return AbstractNetEvent::infoName() + QString("(%1, %2, %3)").arg(id_).arg(time_).arg(value_) + ")";
}

void NetEventUiFloat::serialize(IO::DataStream &io) const
{
    io << time_ << value_ << id_;
}

void NetEventUiFloat::deserialize(IO::DataStream &io)
{
    io >> time_ >> value_ >> id_;
}



} // namespace MO
