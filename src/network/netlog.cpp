/** @file netlog.cpp

    @brief logging for network objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpSocket>
#include <QHostAddress>

#include "netlog.h"
#include "io/application.h"
#include "io/applicationtime.h"
#include "io/currentthread.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

NetworkLogger * NetworkLogger::instance_ = 0;

NetworkLogger::NetworkLogger(QObject * p)
    : QObject   (p),
      acceptedLevels_   (0xffff ^ (EVENT_V2 | DEBUG_V2))
{
    MO_DEBUG_IO("NetworkLogger::NetworkLogger(" << p << ")");
}

NetworkLogger::~NetworkLogger()
{
}

NetworkLogger& NetworkLogger::instance()
{
    if (!instance_)
    {
        MO_ASSERT(application, "NetworkLogger needs Application object");
        instance_ = new NetworkLogger(application);
    }
    return *instance_;
}


void NetworkLogger::connectForLogging(QTcpSocket * socket)
{
    //NetworkLogger & n(instance());

    connect(socket,
        static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
                &QTcpSocket::error), [=]()
    {
        MO_NETLOG(ERROR,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "error: " << socket->errorString());
    });

    connect(socket, &QTcpSocket::hostFound, [=]()
    {
        MO_NETLOG(EVENT,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "host found");
    });

    connect(socket, &QTcpSocket::connected, [=]()
    {
        MO_NETLOG(EVENT,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "connected");
    });

    connect(socket, &QTcpSocket::disconnected, [=]()
    {
        MO_NETLOG(EVENT,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "connection closed");
    });

    connect(socket, &QTcpSocket::readyRead, [=]()
    {
        MO_NETLOG(EVENT,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "received " << socket->bytesAvailable() << " bytes");
    });

    connect(socket, &QTcpSocket::bytesWritten, [=](qint64 bytes)
    {
        MO_NETLOG(EVENT,
                  "TcpSocket(" << socket->peerAddress().toString() << "): "
                  "send " << bytes << " bytes");
    });
}


void NetworkLogger::beginWrite(Level l)
{
    NetworkLogger & n = instance();

    n.curLevel_ = l;

    if (!(l & n.acceptedLevels_))
        return;

    n.stream_.str("");
}

void NetworkLogger::endWrite()
{
    NetworkLogger & n(instance());

    if (!(n.curLevel_ & n.acceptedLevels_))
        return;

    QString linetext = QString::fromStdString(n.stream_.str());

    LogLine line;
    line.level = n.curLevel_;
    line.string =
        currentThreadName() + "/" + applicationTimeString()
            + " " + linetext;

    n.text_.append(line);

    MO_DEBUG("NETLOG: " << linetext);

    emit n.textAdded(line.level, line.string);
}

} // namespace MO
