/** @file tcpclient.cpp

    @brief client for general tcp messages

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.09.2014</p>
*/

#include <QTcpSocket>
#include <QHostAddress>
#include <QHostInfo>

#include "tcpclient.h"
#include "netlog.h"
#include "netevent.h"

namespace MO {

TcpClient::TcpClient(QObject *parent) :
    QObject(parent)
{
    MO_NETLOG(CTOR, "TcpClient::TcpClient(" << parent << ")");
}


bool TcpClient::connectToMaster()
{
    MO_NETLOG(DEBUG, "TcpClient::connectToMaster()");

    auto info = QHostInfo::fromName("matrixoptimizer.master");

    if (info.error() != QHostInfo::NoError)
    {
        MO_NETLOG(ERROR, "TcpClient::connectToMaster() "
                  "host not found");
        return false;
    }

    return true;
}


} // namespace MO
