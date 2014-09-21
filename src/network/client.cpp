/** @file tcpclient.cpp

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QTimer>

#include "client.h"
#include "netlog.h"
#include "netevent.h"
#include "networkmanager.h"

namespace MO {

Client::Client(QObject *parent) :
    QObject (parent),
    socket_ (new QTcpSocket(this)),
    timer_  (new QTimer(this))
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

    timer_->setSingleShot(true);
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer_()));
}

Client::~Client()
{
    if (socket_->isOpen())
        socket_->close();
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

void Client::reconnect_(int ms)
{
    timer_->start(ms);
}

void Client::onTimer_()
{
    connect_();
}

void Client::connectTo(const QString &ip)
{
    connectTo(QHostAddress(ip));
}

void Client::connectTo(const QHostAddress & a)
{
    address_ = a;
    connect_();
}

bool Client::sendEvent(AbstractNetEvent * event)
{
    MO_NETLOG(DEBUG, "Client::sendEvent(" << event << ")");

    if (!socket_->isWritable())
    {
        MO_NETLOG(ERROR, "Client::sendEvent(" << event << ") on unwriteable socket");
        delete event;
        return false;
    }

    bool suc = event->send(socket_);
    delete event;

    return suc;
}

void Client::connect_()
{
    MO_NETLOG(DEBUG, "Client::connect_(" << address_.toString() << ")");

    socket_->connectToHost(address_, NetworkManager::defaultTcpPort());
}

void Client::onError_()
{
    MO_NETLOG(ERROR, "Client: connection error:\n"
              << socket_->errorString());

    reconnect_(1000);
}

void Client::onConnected_()
{
    MO_NETLOG(EVENT, "Client: connected to "
              << address_.toString());

    emit connected();
}

void Client::onDisconnected_()
{
    MO_NETLOG(EVENT, "Client: disconnected from "
              << address_.toString());

    emit disconnected();

    reconnect_(1000);
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
