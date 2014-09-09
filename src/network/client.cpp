/** @file tcpclient.cpp

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>

#include "client.h"
#include "netlog.h"
#include "netevent.h"
#include "networkmanager.h"

namespace MO {

Client::Client(QObject *parent) :
    QObject (parent),
    socket_ (new QTcpSocket(this))
{
    MO_NETLOG(CTOR, "Client::Client(" << parent << ")");

    connect(socket_, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError_()));
    connect(socket_, SIGNAL(connected()),
            this, SLOT(onConnected_()));
    connect(socket_, SIGNAL(disconnected()),
            this, SLOT(onDisconnected_()));
    connect(socket_, SIGNAL(readyRead()),
            this, SLOT(onData_()));
}


bool Client::connectToMaster()
{
    //const QString name = "_tcp.matrixoptimizer.master";
    const QString name = "schleppi";

    MO_NETLOG(DEBUG, "Client::connectToMaster()");

    auto info = QHostInfo::fromName(name);

    if (info.error() != QHostInfo::NoError)
    {
        MO_NETLOG(ERROR, "Client::connectToMaster() "
                  "error resolving hostname '" << name << "'\n"
                  << info.errorString());
        return false;
    }

    if (info.addresses().isEmpty())
    {
        MO_NETLOG(ERROR, "Client::connectToMaster() "
                  "host '" << name << "' not found");
        return false;
    }

    connectTo(info.addresses()[0]);

    return true;
}

void Client::connectTo(const QString &ip)
{
    connectTo(QHostAddress(ip));
}

void Client::connectTo(const QHostAddress & a)
{
    MO_NETLOG(DEBUG, "Client::connectTo(" << a.toString() << ")");

    address_ = a;

    socket_->connectToHost(address_, NetworkManager::defaultTcpPort());
}

void Client::onError_()
{
    MO_NETLOG(ERROR, "Client: connection error:\n"
              << socket_->errorString());
}

void Client::onConnected_()
{
    MO_NETLOG(EVENT, "Client: connected to "
              << address_.toString());
}

void Client::onDisconnected_()
{
    MO_NETLOG(EVENT, "Client: disconnected from "
              << address_.toString());
}

void Client::onData_()
{
    MO_NETLOG(EVENT, "Client: data available ("
              << socket_->bytesAvailable() << "b)");

    AbstractNetEvent * event = AbstractNetEvent::receive(socket_);

    if (event)
    {
        emit eventReceived(event);
    }
}

} // namespace MO
