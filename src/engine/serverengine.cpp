/** @file serverengine.cpp

    @brief Server/client responsibility wraper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include "serverengine.h"
#include "network/tcpserver.h"
#include "network/netevent.h"
#include "network/netlog.h"

namespace MO {


ServerEngine::ServerEngine(QObject *parent)
    : QObject       (parent),
      server_       (new TcpServer(this))
{
    MO_NETLOG(CTOR, "ServerEngine::ServerEngine(" << parent << ")");
}

bool ServerEngine::isRunning() const
{
    return server_->isListening();
}

bool ServerEngine::open()
{
    return server_->open();
}

void ServerEngine::close()
{
    server_->close();
}


} // namespace MO
