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
      stream_   (new QTextStream(&curText_))
{
    MO_DEBUG_IO("NetworkLogger::NetworkLogger(" << p << ")");
}

NetworkLogger::~NetworkLogger()
{
    delete stream_;
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
    n.stream_->seek(0);
    n.curText_.clear();
    n.curLevel_ = l;
}

void NetworkLogger::endWrite()
{
    NetworkLogger & n(instance());

    LogLine line;
    line.level = n.curLevel_;
    line.string =
        currentThreadName() + "/" + applicationTimeString()
            + " " + n.curText_;

    n.text_.append(line);

    MO_DEBUG("NETLOG: " << n.curText_);

    emit n.textAdded(line.level, line.string);
}

} // namespace MO
