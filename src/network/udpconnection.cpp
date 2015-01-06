/** @file UdpConnection.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#include <QUdpSocket>

#include "udpconnection.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {


UdpConnection::UdpConnection(QObject *parent)
    : QObject   (parent),
      ref_      (1),
      socket_   (new QUdpSocket(this))
{
    MO_DEBUG_UDP("UdpConnection::UdpConnection()");

    connect(socket_, SIGNAL(readyRead()), this, SLOT(receive_()));
}

UdpConnection::~UdpConnection()
{
    MO_DEBUG_UDP("UdpConnection::~UdpConnection()");

    close();
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

    addr_ = "any";
    port_ = port;
    return true;
}


void UdpConnection::close()
{
    if (!isOpen())
        return;

}


bool UdpConnection::sendDatagram(const char * data, uint64_t len)
{
    MO_DEBUG_UDP("UdpConnection::sendDatagram(" << data << ", " << len
                 << ", " << addr_.toString() << ":" << port_ << ")");

    int64_t sent = socket_->writeDatagram(data, len, addr_, port_);

    if (sent<0 || sent != (int64_t)len)
    {
        MO_WARNING("udp writing failed: " << socket_->errorString());
        return false;
    }

    return true;
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
    while (socket_->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(socket_->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        socket_->readDatagram(datagram.data(), datagram.size(),
                              &sender, &senderPort);

        MO_DEBUG_UDP("UdpConnection::received " << datagram.size() << " bytes from " << addr_.toString() << ":" << port_ << ")");

        data_.append(datagram);
        emit dataReady();
    }
}


} // namespace MO
