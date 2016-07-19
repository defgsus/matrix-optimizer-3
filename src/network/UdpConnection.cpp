/** @file udpconnection.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#include <QUdpSocket>

#include "UdpConnection.h"
#include "io/error.h"
#include "io/log.h"
#include "network/netlog.h"

namespace MO {


UdpConnection::UdpConnection(QObject *parent)
    : QObject   (parent),
      ref_      (1),
      socket_   (new QUdpSocket(0)),
      port_     (0)
{
    MO_DEBUG_UDP("UdpConnection::UdpConnection()");
    MO_NETLOG(CTOR, "UdpConnection::UdpConnection()");

    connect(socket_, SIGNAL(readyRead()), this, SLOT(receive_()));
}

UdpConnection::~UdpConnection()
{
    MO_DEBUG_UDP("UdpConnection::~UdpConnection()");
    MO_NETLOG(CTOR, "UdpConnection::~UdpConnection()");

    close();

    delete socket_;
}

bool UdpConnection::isOpen() const
{
    return socket_->isOpen();
}

bool UdpConnection::isData() const
{
    return !data_.isEmpty();
}

bool UdpConnection::open(const QHostAddress &addr, uint16_t port)
{
    MO_DEBUG_UDP("UdpConnection::open(" << addr.toString() << ":" << port << ")");

    if (!socket_->bind(addr, port))
    {
        MO_WARNING("UdpConnection::open(" << addr.toString() << ":" << port << ") failed: "
                   << socket_->errorString());
        addr_.clear();
        port_ = 0;
        return false;
    }

    addr_ = addr;
    port_ = port;

    MO_NETLOG(EVENT, "udp connection bound " << addr_.toString() << ":" << port_);

    return true;
}

bool UdpConnection::open(uint16_t port)
{
    MO_DEBUG_UDP("UdpConnection::open(" << port << ")");

    if (!socket_->bind(port, QUdpSocket::ShareAddress))
    {
        MO_WARNING("UdpConnection::open(" << port << ") failed: " << socket_->errorString());
        addr_.clear();
        port_ = 0;
        return false;
    }

    addr_ = socket_->peerAddress();
    port_ = port;

    MO_NETLOG(EVENT, "udp connection bound " << addr_.toString() << ":" << port_);

    return true;
}

bool UdpConnection::openMulticastRead(const QString& addr, uint16_t port)
{
    MO_DEBUG_UDP("UdpConnection::openMulticastRead(" << addr << ":" << port << ")");

    if (!socket_->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress))
    {
        MO_WARNING("UdpConnection: bind(AnyIPv4:" << port << ") failed: "
                   << socket_->errorString());
        addr_.clear();
        port_ = 0;
        return false;
    }

    if (!socket_->joinMulticastGroup(QHostAddress(addr)))
    {
        MO_WARNING("UdpConnection: joinMulticastGroup(" << addr << ") failed: "
                   << socket_->errorString());
        socket_->close();
        addr_.clear();
        port_ = 0;
        return false;
    }

    addr_ = addr;
    port_ = port;

    MO_NETLOG(EVENT, "udp multicast connection bound " << addr_.toString() << ":" << port_);

    return true;
}

void UdpConnection::close()
{
    if (!isOpen())
        return;

}



QByteArray UdpConnection::readData()
{
    if (data_.isEmpty())
        return QByteArray();

    QByteArray d = data_.front();
    data_.pop_front();
    return d;
}

void UdpConnection::receive_()
{
    bool ready = false;
    while (socket_->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(socket_->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        socket_->readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);

        MO_DEBUG_UDP("UdpConnection::received "
                     << datagram.size() << " bytes from "
                     << addr_.toString() << ":" << port_ << "): '"
                     << QString::fromUtf8(datagram) << "'");

        data_.append(datagram);
        ready = true;
    }

    if (ready)
        emit dataReady();
}



bool UdpConnection::sendDatagram(const char * data, uint64_t len)
{
    MO_DEBUG_UDP("UdpConnection::sendDatagram(" << data << ", " << len
                 << ", " << addr_.toString() << ":" << port_ << ")");

    int64_t sent = socket_->writeDatagram(data, len, addr_, port_);

    if (sent<0 || sent != (int64_t)len)
    {
        MO_NETLOG(WARNING, "udp writing failed for " << addr_.toString() << ":" << port_
                  << ": " << socket_->errorString());
        return false;
    }

    return true;
}

bool UdpConnection::sendDatagram(const QByteArray& data)
{
    MO_DEBUG_UDP("UdpConnection::sendDatagram(size=" << data.size()
                 << ", " << addr_.toString() << ":" << port_ << ")");

    int64_t sent = socket_->writeDatagram(data, addr_, port_);

    if (sent<0 || sent != (int64_t)data.length())
    {
        MO_NETLOG(WARNING, "udp writing failed for " << addr_.toString() << ":" << port_
                  << ": " << socket_->errorString());
        return false;
    }

    return true;
}

bool UdpConnection::sendDatagram(const QByteArray& data, const QHostAddress& addr, uint16_t port)
{
    MO_DEBUG_UDP("UdpConnection::sendDatagram(size=" << data.size()
                 << ", " << addr.toString() << ":" << port << ")");

    int64_t sent = socket_->writeDatagram(data, addr, port);

    if (sent<0 || sent != (int64_t)data.length())
    {
        MO_NETLOG(WARNING, "udp writing failed for " << addr.toString() << ":" << port
                  << ": " << socket_->errorString());
        return false;
    }

    return true;
}

bool UdpConnection::sendDatagram(const char * data, uint64_t len, const QHostAddress& addr, uint16_t port)
{
    MO_DEBUG_UDP("UdpConnection::sendDatagram(" << data << ", " << len
                 << ", " << addr.toString() << ":" << port << ")");

    int64_t sent = socket_->writeDatagram(data, len, addr, port);

    if (sent<0 || sent != (int64_t)len)
    {
        MO_NETLOG(WARNING, "udp writing failed for " << addr.toString() << ":" << port
                  << ": " << socket_->errorString());
        return false;
    }

    return true;
}


} // namespace MO
