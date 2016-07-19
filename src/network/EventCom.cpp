/** @file eventcom.cpp

    @brief NetEvent sender/receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/22/2014</p>
*/

#include <QAbstractSocket>

#include "EventCom.h"
#include "io/error.h"
#include "io/DataStream.h"
#include "network/NetEvent.h"
#include "network/netlog.h"

namespace MO {



EventCom::EventCom(QObject *parent)
    : QObject       (parent),
      bufferSize_   (0)
{
    MO_NETLOG(CTOR, "EventCom::EventCom(" << parent << ")");
}

bool EventCom::sendEvent(QAbstractSocket * socket, AbstractNetEvent * event)
{
    MO_NETLOG(EVENT, "EventCom::sendEvent(" << socket->peerName()
              << ", " << event->infoName() << " )");

    if (socket)
        event->socket_ = socket;

    if (!event->socket_)
    {
        MO_NETLOG(ERROR, "Can't send NetEvent '" << event->className() << "' without socket");
        return false;
    }

    if (!event->socket_->isWritable())
    {
        MO_NETLOG(ERROR, "Attempt to send NetEvent '" << event->className() << "' on unwriteable socket");
        return false;
    }

    try
    {
        // first serialize into a bytearray
        QByteArray data;
        IO::DataStream s(&data, QIODevice::WriteOnly);

        s.writeHeader("netev", 1);

        s << event->className();
        s << event->counter();

        event->serialize(s);

        // then write the size + raw data to socket/device
        QDataStream stream(event->socket_);
        stream << (qint64)data.size();

        qint64 written = event->socket_->write(data);

        if (written != data.size())
            MO_IO_ERROR(WRITE, "Could not write all of NetEvent data to socket "
                        << written << "/" << data.size());

        return event->isSend_ = true;
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Error sending event " << event->infoName() << "\n" << e.what());
    }
    return false;

}

void EventCom::inputData(QAbstractSocket * socket)
{
    // nothing in buffer?
    if (bufferSize_ == 0)
    {
        // read size (suppose beginning of packet)
        qint64 size;
        {
            QDataStream stream(socket);
            stream >> size;
        }

        // read data
        QByteArray data = socket->readAll();

        if (data.isEmpty())
        {
            MO_NETLOG(ERROR, "EventCom: received NULL package");
            return;
        }

        while (data.size() && data.size() >= size)
        {
            // read exactly one packet?
            if (data.size() == size)
            {
                processPacket_(data, socket);
                return;
            }
            // otherwise we got more than one packet?

            MO_NETLOG(DEBUG_V2, "got more than one packet " << size << "/" << data.size());

            // process first part
            processPacket_(data.left(size), socket);
            // get rest
            data.remove(0, size);
            // read size of next chunk
            QDataStream stream(data);
            stream >> size;
            data.remove(0, sizeof(qint64));
//            MO_NETLOG(DEBUG, "next chunk " << size << "/" << data.size());
        }

        // got only a part?
        if (data.size() != 0 && data.size() < size)
        {
            MO_NETLOG(DEBUG_V2, "got part of a packet " << data.size() << "/" << size);
            // add to buffer
            bufferSize_ = size;
            buffer_ = data;
        }
    }

    // process buffered data
    if (bufferSize_)
    {
        MO_NETLOG(DEBUG_V2, "processing data in buffer " << buffer_.size() << "/" << bufferSize_);

        // append data to buffer
        buffer_.append(socket->readAll());

        // got more than one packet?
        while (buffer_.size() && buffer_.size() >= bufferSize_)
        {
            // read exactly one packet?
            if (buffer_.size() == bufferSize_)
            {
                processPacket_(buffer_, socket);
                bufferSize_ = 0;
                buffer_.clear();
                return;
            }

            MO_NETLOG(DEBUG_V2, "got more than one packet in buffer "
                      << bufferSize_ << "/" << buffer_.size());

            // process first part
            processPacket_(buffer_.left(bufferSize_), socket);
            // get rest
            buffer_.remove(0, bufferSize_);
            // read size of next chunk
            QDataStream stream(buffer_);
            stream >> bufferSize_;
            buffer_.remove(0, sizeof(qint64));
        }

        // rest of the buffer will be processed later

    }
}

void EventCom::processPacket_(const QByteArray & data, QAbstractSocket * sender)
{
    AbstractNetEvent * event = 0;
    QString cname;

    IO::DataStream stream(data);

    try
    {
        stream.readHeader("netev", 1);

        // get classname
        stream >> cname;

        event = AbstractNetEvent::createClass(cname);
        if (!event)
        {
            MO_NETLOG(ERROR, "received unknown NetEvent '" << cname << "'");
            return;
        }

        stream >> event->counter_;

        event->deserialize(stream);
        event->socket_ = sender;
        event->isReceived_ = true;

        MO_NETLOG(EVENT, "EventCom::eventReceived(" << sender->peerName()
                  << ", " << event->infoName() << ", size = " << data.size() << ")");

        emit eventReceived(event);
    }
    catch (const Exception & e)
    {
        MO_NETLOG(ERROR, "error on receiving NetEvent '" << cname << "'\n"
                  << e.what());

        delete event;
    }
}



} // namespace MO
