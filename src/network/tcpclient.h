/** @file tcpclient.h

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#ifndef MOSRC_NETWORK_TCPCLIENT_H
#define MOSRC_NETWORK_TCPCLIENT_H

#include <QObject>

namespace MO {

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);

signals:

public slots:

    bool connectToMaster();

};

} // namespace MO

#endif // MOSRC_NETWORK_TCPCLIENT_H
