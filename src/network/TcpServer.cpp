/** @file tcpserver.cpp

    @brief Tcp connection listener

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpServer>
#include <QTcpSocket>

#include "TcpServer.h"
#include "io/error.h"
#include "netlog.h"
#include "NetworkManager.h"

namespace MO {

TcpServer::TcpServer(QObject *parent)
    : QObject   (parent),
      server_   (new QTcpServer(this))
{
    MO_NETLOG(CTOR, "TcpServer::TcpServer(" << parent << ")");
}

TcpServer::~TcpServer()
{
    MO_NETLOG(CTOR, "TcpServer::~TcpServer()");

    if (isListening())
        close();
}

bool TcpServer::isListening() const
{
    return server_->isListening();
}

bool TcpServer::open()
{
    int port = NetworkManager::defaultTcpPort();

    MO_NETLOG(DEBUG, "TcpServer: start listening on port " << port);

    if (!server_->listen(QHostAddress::Any//("127.0.0.1")
                         , port))
    {
        MO_NETLOG(ERROR, "TcpServer: failed to start: " << server_->errorString());
        return false;
    }

    MO_NETLOG(EVENT, "TcpServer: started listening on "
              << server_->serverAddress().toString()
              << ":" << server_->serverPort());

    connect(server_, SIGNAL(newConnection()), this, SLOT(onNewConnection_()));
    connect(server_, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            this, SLOT(onAcceptError_(QAbstractSocket::SocketError)));

    return true;
}

void TcpServer::close()
{
    MO_NETLOG(DEBUG, "TcpServer::close()");

    server_->close();
}

const QString& TcpServer::socketName(QTcpSocket * s) const
{
    static const QString unknown = tr("unknown");

    auto it = sockets_.find(s);
    return it == sockets_.end() ? unknown : it.value();
}

void TcpServer::onAcceptError_(QAbstractSocket::SocketError )
{
    MO_NETLOG(ERROR, "TcpServer: error on accept-connection: " << server_->errorString());
    server_->resumeAccepting();
}

void TcpServer::onNewConnection_()
{
    QTcpSocket * socket = server_->nextPendingConnection();

    if (!socket)
    {
        MO_NETLOG(ERROR, "TcpServer: connection request without a socket!");
        return;
    }

    const QString name = socket->peerAddress().toString();

    MO_NETLOG(EVENT, "TcpServer: new connection from " << name);

    // stash away
    sockets_.insert(socket, name);

    // connect

    connect(socket,
        static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
                &QTcpSocket::error), [=]()
    {
        MO_NETLOG(ERROR, "TcpServer: connection error: "
                  << socket->errorString() << " from " << socketName(socket));
        emit socketError(socket);
    });

    connect(socket, &QTcpSocket::readyRead, [=]()
    {
        MO_NETLOG(EVENT_V2, "TcpServer: received " << socket->bytesAvailable()
                  << " bytes from " << socketName(socket));
        emit socketData(socket);
    });

    connect(socket, &QTcpSocket::bytesWritten, [=](qint64 bytes)
    {
        MO_NETLOG(EVENT_V2, "TcpServer: send " << bytes
                  << " bytes to " << socketName(socket));
        emit socketDataWritten(socket, bytes);
    });

    connect(socket, &QTcpSocket::disconnected, [=]()
    {
        MO_NETLOG(EVENT, "TcpServer: connection "
                  << socketName(socket) << " closed");

        // remove from list
        sockets_.remove(socket);

        // tell others
        emit socketClosed(socket);
    });

    // emit connection
    emit socketOpened(socket);
}


} // namespace MO
