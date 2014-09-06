/** @file tcpserver.h

    @brief Tcp listener for matrixoptimizer clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_NETWORK_TCPSERVER_H
#define MOSRC_NETWORK_TCPSERVER_H

#include <QObject>

namespace MO {

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

signals:

public slots:

};

} // namespace MO

#endif // MOSRC_NETWORK_TCPSERVER_H
