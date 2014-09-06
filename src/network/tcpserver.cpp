/** @file tcpserver.cpp

    @brief Tcp listener for matrixoptimizer clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpServer>
#include <QTcpSocket>

#include "tcpserver.h"
#include "io/error.h"
#include "netlog.h"
#include "networkmanager.h"

namespace MO {

TcpServer::TcpServer(QObject *parent)
    : QObject   (parent),
      server_   (new QTcpServer(this))
{
    MO_NETLOG(CTOR, "TcpServer::TcpServer(" << parent << ")");
}

bool TcpServer::isListening() const
{
    return server_->isListening();
}

bool TcpServer::open()
{
    int port = NetworkManager::defaultTcpPort();

    MO_NETLOG(DEBUG, "TcpServer: start listening on port " << port);

    if (!server_->listen())
    {
        MO_NETLOG(ERROR, "TcpServer: failed to start: " << server_->errorString());
        return false;
    }

    MO_NETLOG(EVENT, "TcpServer: started listening on port " << port);

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

    const QString name = socket->peerName() + ":" + socket->peerPort();

    MO_NETLOG(EVENT, "TcpServer: new connection from " << name);

    // stash away
    sockets_.insert(socket, name);

    // connect

    connect(socket,
        static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
                &QTcpSocket::error), [=]()
    {
        MO_NETLOG(ERROR, "TcpServer:: connection error: "
                  << socket->errorString());
        emit socketError(socket);
    });

    connect(socket, &QTcpSocket::readyRead, [=]()
    {
        emit socketData(socket);
    });

    connect(socket, &QTcpSocket::bytesWritten(qint64), [=](qint64 bytes)
    {
        emit socketDataWritten(socket, bytes);
    });

    connect(socket, &QTcpSocket::disconnected, [=]()
    {
        MO_NETLOG(EVENT, "TcpServer: connection "
                  << sockets_[socket] << " closed");

        // remove from list
        sockets_.remove(socket);

        // tell others
        emit socketClosed(socket);
    });

    // emit connection
    emit socketOpened(socket);
}


} // namespace MO
