/** @file tcpclient.h

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#ifndef MOSRC_NETWORK_TCPCLIENT_H
#define MOSRC_NETWORK_TCPCLIENT_H

#include <QObject>
#include <QHostAddress>

class QTcpSocket;

namespace MO {

class AbstractNetEvent;

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);

signals:

    void eventReceived(AbstractNetEvent * event);

public slots:

    bool connectToMaster();

    void connectTo(const QString& ip);
    void connectTo(const QHostAddress&);

private slots:

    void onError_();
    void onConnected_();
    void onDisconnected_();
    void onData_();

private:


    QHostAddress address_;
    QTcpSocket * socket_;
};

} // namespace MO

#endif // MOSRC_NETWORK_TCPCLIENT_H
