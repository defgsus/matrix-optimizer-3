/** @file UdpConnection.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#include <QUdpSocket>

#ifndef MOSRC_NETWORK_UdpConnection_H
#define MOSRC_NETWORK_UdpConnection_H

#include <QObject>


#if 1
#   define MO_DEBUG_UDP(arg__) MO_DEBUG(arg__)
#else
#   define MO_DEBUG_UDP(unused__)
#endif


namespace MO {

class UdpConnection : public QObject
{
    Q_OBJECT
    ~UdpConnection();
public:
    explicit UdpConnection(QObject *parent = 0);

    void addRef() { ref_++; }
    void releaseRef() { if (--ref_ == 0) delete this; }

    // -------------- getter --------

    bool isOpen() const;
    bool isData() const;

    const QHostAddress& address() const { return addr_; }
    uint16_t port() const { return port_; }

    /** Pulls new data packets as long as isData() returns true */
    QByteArray readData();

signals:

    /** Ready to read */
    void dataReady();

public slots:

    bool open(const QHostAddress& addr, uint16_t port);

    /** Starts listening on port for any incomming datagrams */
    bool open(uint16_t port);

    /** Open for multicast receiving from group address @p addr */
    bool openMulticastRead(const QString& groupAddress, uint16_t port);

    void close();

    /** Sends to the default location specified with open() */
    bool sendDatagram(const QByteArray&);
    /** Sends to the default location specified with open() */
    bool sendDatagram(const char * data, uint64_t len);
    /** Sends to the specified address and port */
    bool sendDatagram(const char * data, uint64_t len, const QHostAddress& addr, uint16_t port);


private slots:

    void receive_();

private:

    int ref_;
    QUdpSocket * socket_;
    QHostAddress addr_;
    uint16_t port_;
    QList<QByteArray> data_;
};


} // namespace MO


#endif // MOSRC_NETWORK_UdpConnection_H
