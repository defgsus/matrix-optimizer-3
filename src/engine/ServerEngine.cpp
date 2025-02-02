/** @file serverengine.cpp

    @brief Server/client responsibility wraper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include <QTcpSocket>

#include "ServerEngine.h"
#include "network/TcpServer.h"
#include "network/NetEvent.h"
#include "network/netlog.h"
#include "network/EventCom.h"
#include "network/UdpAudioConnection.h"
#include "io/Application.h"
#include "io/Settings.h"
#include "io/FileManager.h"
#include "tool/deleter.h"

namespace MO {

ServerEngine & serverEngine()
{
    static ServerEngine * instance_ = 0;
    if (!instance_)
        instance_ = new ServerEngine(application());

    return *instance_;
}


struct ClientInfo::Private
{

};



ServerEngine::ServerEngine(QObject *parent)
    : QObject       (parent),
      server_       (new TcpServer(this)),
      eventCom_     (new EventCom(this)),
      audioOut_     (0)
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

    connect(eventCom_, SIGNAL(eventReceived(AbstractNetEvent*)),
            this, SLOT(onEventCom_(AbstractNetEvent*)));
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
#if 0
UdpAudioConnection * ServerEngine::getAudioOutStream()
{
    if (!audioOut_)
    {
        audioOut_ = new UdpAudioConnection();
        // XXX link
    }

    return audioOut_;
}
#endif

void ServerEngine::close()
{
    sendClose_();
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

bool ServerEngine::sendEvent(AbstractNetEvent * e)
{
    MO_NETLOG(DEBUG, "ServerEngine::sendEvent( " << e->infoName() << " )");

    ScopedDeleter<AbstractNetEvent> deleter(e);

    bool suc = !clients_.isEmpty();

    for (ClientInfo& i : clients_)
    {
        suc &= eventCom_->sendEvent(i.tcpSocket, e);
    }

    return suc;
}

bool ServerEngine::sendEvent(ClientInfo& client, AbstractNetEvent * e)
{
    MO_NETLOG(DEBUG, "ServerEngine::sendEvent(" << client.index << ", " << e->infoName() << " )");

    ScopedDeleter<AbstractNetEvent> deleter(e);

    return eventCom_->sendEvent(client.tcpSocket, e);
}

void ServerEngine::sendProjectionSettings()
{
    auto event = new NetEventRequest;
    event->setRequest(NetEventRequest::SET_PROJECTION_SETTINGS);

    auto s = settings()->getDefaultProjectionSettings();
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
    eventCom_->sendEvent(inf.tcpSocket, &r);
}

void ServerEngine::getClientIndex_(ClientInfo & inf)
{
    // construct event
    NetEventRequest r;
    r.setRequest(NetEventRequest::GET_CLIENT_STATE);

    // send off to client
    eventCom_->sendEvent(inf.tcpSocket, &r);
}

void ServerEngine::sendProjectionSettings_(ClientInfo & inf)
{
    NetEventRequest event;
    event.setRequest(NetEventRequest::SET_PROJECTION_SETTINGS);

    auto s = settings()->getDefaultProjectionSettings();
    QByteArray data;
    s.serialize(data);

    // XXX error checking???
    eventCom_->sendEvent(inf.tcpSocket, &event);
}

void ServerEngine::onTcpData_(QTcpSocket * s)
{
    // check for event

    eventCom_->inputData(s);
}

void ServerEngine::onEventCom_(AbstractNetEvent * event)
{
    // find client

    const int idx = clientForTcp_(static_cast<QTcpSocket*>(event->sender()));
    if (idx<0)
    {
        MO_NETLOG(WARNING, "data from unknown client " << event->sender()->peerName());
        return;
    }
    ClientInfo& client = clients_[idx];

    onEvent_(client, event);
}

void ServerEngine::onEvent_(ClientInfo & client, AbstractNetEvent * event)
{
    //MO_NETLOG(DEBUG, "ServerEngine::onEvent_(" << client.index << ", " << event->infoName() << " )");

    ScopedDeleter<AbstractNetEvent> deleter(event);

    if (NetEventSysInfo * sys = netevent_cast<NetEventSysInfo>(event))
    {
        client.sysinfo = sys->info();
        return;
    }

    if (NetEventLog * log = netevent_cast<NetEventLog>(event))
    {
        emit clientMessage(client, log->level(), log->message());
        return;
    }

    if (NetEventClientState * state = netevent_cast<NetEventClientState>(event))
    {
        client.state = state->state();
        client.index = client.state.clientIndex();
        emit clientStateChanged(client.index);
        return;
    }

    if (NetEventRequest * req = netevent_cast<NetEventRequest>(event))
    {
        if (req->request() == NetEventRequest::GET_SERVER_FILE_TIME)
        {
            auto f = req->createResponse<NetEventFileInfo>();
            /** @note We load the local file but keep the absolute real filename
                as identifier for the file. */
            f->setFilename( IO::FileManager().localFilename(req->data().toString()) );
            f->getFileTime();
            f->setFilename(req->data().toString());

            sendEvent(client, f);
            return;
        }

        if (req->request() == NetEventRequest::GET_SERVER_FILE)
        {
            auto f = req->createResponse<NetEventFile>();
            /** @note We load the local file but keep the absolute real filename
                as identifier for the file. */
            f->loadFile( IO::FileManager().localFilename(req->data().toString()) );
            f->setFilename( req->data().toString() );

            sendEvent(client, f);
            return;
        }
    }

    MO_NETLOG(WARNING, "unhandled NetEvent '" << event->className()
              << "' from client " << client.tcpSocket->peerName());
}


void ServerEngine::showInfoWindow(int index, bool show)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(show? NetEventRequest::SHOW_INFO_WINDOW
                     : NetEventRequest::HIDE_INFO_WINDOW);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}

void ServerEngine::showRenderWindow(int index, bool show)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(show? NetEventRequest::SHOW_RENDER_WINDOW
                     : NetEventRequest::HIDE_RENDER_WINDOW);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}

void ServerEngine::setClientIndex(int index, int cindex)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(NetEventRequest::SET_CLIENT_INDEX);
    r.setData(cindex);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}

void ServerEngine::getClientState(int index)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(NetEventRequest::GET_CLIENT_STATE);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}

void ServerEngine::setDesktopIndex(int index, int desk)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(NetEventRequest::SET_DESKTOP_INDEX);
    r.setData(desk);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}


void ServerEngine::sendClearFileCache(int index)
{
    if (index >= clients_.size())
        return;

    NetEventRequest r;
    r.setRequest(NetEventRequest::CLEAR_FILE_CACHE);

    eventCom_->sendEvent(clients_[index].tcpSocket, &r);
}

void ServerEngine::sendClose_()
{
    auto e = new NetEventRequest;
    e->setRequest(NetEventRequest::CLOSE_CONNECTION);

    sendEvent(e);
}

bool ServerEngine::sendScene(Scene *scene)
{
    if (!numClients())
        return false;

    auto e = new NetEventScene();
    if (!e->setScene(scene))
    {
        MO_NETLOG(ERROR, "Could not serialize scene");
        return false;
    }

    return sendEvent(e);
}

bool ServerEngine::setScenePlaying(bool enabled)
{
    if (!numClients())
        return false;

    auto r = new NetEventRequest();
    if (enabled)
        r->setRequest(NetEventRequest::START_RENDER);
    else
        r->setRequest(NetEventRequest::STOP_RENDER);

    /** @todo send start/stop timestamp */

    return sendEvent(r);
}

bool ServerEngine::sendAudioConfig(const AUDIO::Configuration& c)
{
    if (!numClients())
        return false;

    auto e = new NetEventAudioConfig();
    e->setConfig(c);

    return sendEvent(e);
}



ProjectionSystemSettings ServerEngine::createProjectionSystemSettings()
{
    ProjectionSystemSettings set;

    for (int i = 0; i < clients_.size(); ++i)
    {
        const ClientInfo & client = clients_[i];

        // the set desktop
        int desktop = client.state.desktop();
        // resolution
        QSize size = client.sysinfo.resolution(desktop);

        // create a projector
        ProjectorSettings proj;
        proj.setName(client.tcpSocket->peerName());
        proj.setWidth(size.width());
        proj.setHeight(size.height());

        set.appendProjector(proj);

        // create camera
        CameraSettings cam;
        cam.setWidth(size.width());
        cam.setHeight(size.height());

        set.setCameraSettings(set.numProjectors()-1, cam);
    }

    return set;
}


} // namespace MO
