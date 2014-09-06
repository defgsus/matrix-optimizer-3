/** @file tcpserver.cpp

    @brief Tcp listener for matrixoptimizer clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpServer>

#include "tcpserver.h"
#include "io/error.h"
#include "netlog.h"

namespace MO {


TcpServer::TcpServer(QObject *parent) :
    QObject(parent)
{
    MO_NETLOG(CTOR, "TcpServer::TcpServer(" << parent << ")");
}


} // namespace MO
