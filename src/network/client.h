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
class QTimer;

namespace MO {

class AbstractNetEvent;

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    ~Client();

signals:

    void connected();
    void disconnected();

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

    void onTimer_();

private:

    /** Reconnect in a second or two */
    void reconnect_(int millisecs);
    void connect_();

    QHostAddress address_;
    QTcpSocket * socket_;

    QTimer * timer_;
};

} // namespace MO

#endif // MOSRC_NETWORK_TCPCLIENT_H
