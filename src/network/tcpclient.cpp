/** @file tcpclient.cpp

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>

#include "tcpclient.h"
#include "netlog.h"
#include "netevent.h"
#include "networkmanager.h"

namespace MO {

TcpClient::TcpClient(QObject *parent) :
    QObject (parent),
    socket_ (new QTcpSocket(this))
{
    MO_NETLOG(CTOR, "TcpClient::TcpClient(" << parent << ")");

    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError_()));
    connect(socket_, SIGNAL(connected()),
            this, SLOT(onConnected_()));
    connect(socket_, SIGNAL(disconnected()),
            this, SLOT(onDisconnected_()));
    connect(socket_, SIGNAL(readyRead()),
            this, SLOT(onData_()));
}


bool TcpClient::connectToMaster()
{
    //const QString name = "_tcp.matrixoptimizer.master";
    const QString name = "schleppi";

    MO_NETLOG(DEBUG, "TcpClient::connectToMaster()");

    auto info = QHostInfo::fromName(name);

    if (info.error() != QHostInfo::NoError)
    {
        MO_NETLOG(ERROR, "TcpClient::connectToMaster() "
                  "error resolving hostname '" << name << "'\n"
                  << info.errorString());
        return false;
    }

    if (info.addresses().isEmpty())
    {
        MO_NETLOG(ERROR, "TcpClient::connectToMaster() "
                  "host '" << name << "' not found");
        return false;
    }

    connectTo(info.addresses()[0]);

    return true;
}

void TcpClient::connectTo(const QString &ip)
{
    connectTo(QHostAddress(ip));
}

void TcpClient::connectTo(const QHostAddress & a)
{
    MO_NETLOG(DEBUG, "TcpClient::connectTo(" << a.toString() << ")");

    address_ = a;

    socket_->connectToHost(address_, NetworkManager::defaultTcpPort());
}

void TcpClient::onError_()
{
    MO_NETLOG(ERROR, "TcpClient: connection error:\n"
              << socket_->errorString());
}

void TcpClient::onConnected_()
{
    MO_NETLOG(EVENT, "TcpClient: connected to "
              << address_.toString());
}

void TcpClient::onDisconnected_()
{
    MO_NETLOG(EVENT, "TcpClient: disconnected from "
              << address_.toString());
}

void TcpClient::onData_()
{
    MO_NETLOG(EVENT, "TcpClient: data available ("
              << socket_->bytesAvailable() << "b)");

    AbstractNetEvent * event = AbstractNetEvent::receive(socket_);

    if (event)
    {
        emit eventReceived(event);
    }
}

} // namespace MO
