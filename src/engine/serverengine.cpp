/** @file serverengine.cpp

    @brief Server/client responsibility wraper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include <QTcpSocket>

#include "serverengine.h"
#include "network/tcpserver.h"
#include "network/netevent.h"
#include "network/netlog.h"
#include "io/application.h"
#include "io/settings.h"
#include "projection/projectionsystemsettings.h"

namespace MO {

ServerEngine & serverEngine()
{
    static ServerEngine * instance_ = 0;
    if (!instance_)
        instance_ = new ServerEngine(application);

    return *instance_;
}


struct ClientInfo::Private
{

};



ServerEngine::ServerEngine(QObject *parent)
    : QObject       (parent),
      server_       (new TcpServer(this))
{
    MO_NETLOG(CTOR, "ServerEngine::ServerEngine(" << parent << ")");

    connect(server_, SIGNAL(socketOpened(QTcpSocket*)),
            this, SLOT(onTcpConnected_(QTcpSocket*)));
    connect(server_, SIGNAL(socketClosed(QTcpSocket*)),
            this, SLOT(onTcpDisconnected_(QTcpSocket*)));
    connect(server_, SIGNAL(socketError(QTcpSocket*)),
            this, SLOT(onTcpError_(QTcpSocket*)));
    connect(server_, SIGNAL(socketData(QTcpSocket*)),
            this, SLOT(onTcpData_(QTcpSocket*)));

}

ServerEngine::~ServerEngine()
{
    for (auto &i : clients_)
        delete i.p_;
}

bool ServerEngine::isRunning() const
{
    return server_->isListening();
}

int ServerEngine::numClients() const
{
    return clients_.size();
}

bool ServerEngine::open()
{
    return server_->open();
}

void ServerEngine::close()
{
    server_->close();

    for (auto &i : clients_)
        delete i.p_;
    clients_.clear();

    emit numberClientsChanged(0);
}

int ServerEngine::clientForTcp_(QTcpSocket * s)
{
   for (int i=0; i<clients_.size(); ++i)
       if (clients_[i].tcpSocket == s)
           return i;
   return -1;
}

void ServerEngine::onTcpConnected_(QTcpSocket * s)
{
    int i = clientForTcp_(s);
    if (i>=0)
    {
        MO_NETLOG(WARNING, "reconnection of already mapped client " << clients_[i].index);
    }
    else
    {
        ClientInfo inf;
        inf.tcpSocket = s;
        inf.index = -1;
        inf.p_ = new ClientInfo::Private();
        clients_.append(inf);
        i = clients_.size()-1;
    }

    // request runtime info

    if (!clients_[i].sysinfo.isValid())
        getSysInfo_(clients_[i]);

    if (clients_[i].index < 0)
        getClientIndex_(clients_[i]);

    // tell others
    emit numberClientsChanged(clients_.size());
}

void ServerEngine::onTcpDisconnected_(QTcpSocket * s)
{
    int i = clientForTcp_(s);
    if (i<0)
    {
        QString str;
        QTextStream stream(&str);
        stream << "current list:";
        for (int i=0; i<clients_.size(); ++i)
            stream << "\n" << i << " " << clients_[i].tcpSocket
                    << " " << clients_[i].tcpSocket->peerName();

        MO_NETLOG(WARNING, "disconnection of unmapped client " << s->peerName()
                  << " " << s << "\n" << str);

    }
    else
    {
        delete clients_[i].p_;
        clients_.removeAt(i);
    }

    emit numberClientsChanged(clients_.size());
}

void ServerEngine::onTcpError_(QTcpSocket * )
{
    //if (s->error() == QAbstractSocket::RemoteHostClosedError)
    //    onTcpDisconnected_(s);
}

void ServerEngine::sendEvent(AbstractNetEvent * e)
{
    for (ClientInfo& i : clients_)
    {
        e->send(i.tcpSocket);
    }

    delete e;
}

void ServerEngine::sendProjectionSettings()
{
    auto event = new NetEventRequest;
    event->setRequest(NetEventRequest::SET_PROJECTION_SETTINGS);

    auto s = settings->getDefaultProjectionSettings();
    QByteArray data;
    s.serialize(data);

    event->setData(data);

    sendEvent(event);
}

void ServerEngine::getSysInfo_(ClientInfo & inf)
{
    // construct event
    NetEventRequest r;
    r.setRequest(NetEventRequest::GET_SYSTEM_INFO);

    // send off to client
    r.send(inf.tcpSocket);
}

void ServerEngine::getClientIndex_(ClientInfo & inf)
{
    // construct event
    NetEventRequest r;
    r.setRequest(NetEventRequest::GET_CLIENT_INDEX);

    // send off to client
    r.send(inf.tcpSocket);
}

void ServerEngine::sendProjectionSettings_(ClientInfo & inf)
{
    NetEventRequest event;
    event.setRequest(NetEventRequest::SET_PROJECTION_SETTINGS);

    auto s = settings->getDefaultProjectionSettings();
    QByteArray data;
    s.serialize(data);

    // XXX error checking???
    event.send(inf.tcpSocket);
}

void ServerEngine::onTcpData_(QTcpSocket * s)
{
    // find client

    const int idx = clientForTcp_(s);
    if (idx<0)
    {
        MO_NETLOG(WARNING, "data from unknown client " << s->peerAddress().toString());
        return;
    }
    ClientInfo& client = clients_[idx];

    // check for event

    AbstractNetEvent * event = AbstractNetEvent::receive(s);

    if (!event)
    {
        MO_NETLOG(WARNING, "unhandled data from client " << s->peerAddress().toString());
        return;
    }

    // handle events

    if (NetEventSysInfo * sys = netevent_cast<NetEventSysInfo>(event))
    {
        client.sysinfo = sys->info();
        return;
    }

    if (NetEventInfo * info = netevent_cast<NetEventInfo>(event))
    {
        if (info->request() == NetEventRequest::GET_CLIENT_INDEX)
            client.index = info->data().toInt();

        return;
    }

    MO_NETLOG(WARNING, "unhandled NetEvent '" << event->className()
              << "' from client " << s->peerName());
}


void ServerEngine::showInfoWindow(int index, bool show)
{
    NetEventRequest r;
    r.setRequest(show? NetEventRequest::SHOW_INFO_WINDOW : NetEventRequest::HIDE_INFO_WINDOW);

    r.send(clients_[index].tcpSocket);
}

void ServerEngine::setClientIndex(int index, int cindex)
{
    NetEventRequest r;
    r.setRequest(NetEventRequest::SET_CLIENT_INDEX);
    r.setData(cindex);

    r.send(clients_[index].tcpSocket);
}

} // namespace MO
