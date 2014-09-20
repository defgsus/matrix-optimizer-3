/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpSocket>

#include "clientengine.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"
#include "network/tcpserver.h"
#include "network/netlog.h"
#include "network/networkmanager.h"
#include "network/netevent.h"
#include "network/client.h"
#include "io/systeminfo.h"
#include "gui/infowindow.h"
#include "io/settings.h"
#include "projection/projectionsystemsettings.h"

namespace MO {

ClientEngine::ClientEngine(QObject *parent) :
    QObject     (parent),
    glManager_  (0),
    infoWindow_ (0),
    client_     (0)
{
}

int ClientEngine::run(int , char ** )
{
    MO_PRINT(tr("Matrix Optimizer Client"));

//    createObjects_();

    //send_ = (argc>1 && QString("send") == argv[1]);

    startNetwork_();

    int ret = application->exec();

    shutDown_();

    return ret;
}

void ClientEngine::shutDown_()
{
    delete glManager_;
    delete infoWindow_;
}


void ClientEngine::createGlObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);

    glWindow_->show();//FullScreen();
}

void ClientEngine::startNetwork_()
{
    client_ = new Client(this);

    connect(client_, SIGNAL(eventReceived(AbstractNetEvent*)), this, SLOT(onNetEvent_(AbstractNetEvent*)));

    client_->connectTo("192.168.1.33");
}

void ClientEngine::showInfoWindow_(bool enable)
{
    if (enable)
    {
        if (!infoWindow_)
            infoWindow_ = new GUI::InfoWindow();

        infoWindow_->updateInfo();
        infoWindow_->showFullScreen();
    }
    else if (infoWindow_)
            infoWindow_->hide();
}

void ClientEngine::onNetEvent_(AbstractNetEvent * event)
{
    if (NetEventRequest * e = netevent_cast<NetEventRequest>(event))
    {
        // respond with system information
        if (e->request() == NetEventRequest::GET_SYSTEM_INFO)
        {
            auto r = e->createResponse<NetEventSysInfo>();
            r->getInfo();
            r->send();
            return;
        }

        if (e->request() == NetEventRequest::GET_CLIENT_INDEX)
        {
            auto r = e->createResponse<NetEventInfo>();
            r->setRequest(e->request());
            r->setData(settings->clientIndex());
            r->send();
            return;
        }

        if (e->request() == NetEventRequest::SET_CLIENT_INDEX)
        {
            MO_NETLOG(EVENT, "setting client index to " << e->data().toInt());
            settings->setClientIndex(e->data().toInt());
            return;
        }

        if (e->request() == NetEventRequest::SHOW_INFO_WINDOW)
        {
            showInfoWindow_(true);
            return;
        }

        if (e->request() == NetEventRequest::HIDE_INFO_WINDOW)
        {
            showInfoWindow_(false);
            return;
        }

        if (e->request() == NetEventRequest::SET_PROJECTION_SETTINGS)
        {
            setProjectionSettings_(e);
            return;
        }
    }

    MO_NETLOG(WARNING, "unhandled NetEvent " << event->className() << " in ClientEngine");
}

void ClientEngine::setProjectionSettings_(NetEventRequest * e)
{
    QByteArray data = e->data().toByteArray();

    ProjectionSystemSettings s;

    try
    {
        s.deserialize(data);
        settings->setDefaultProjectionSettings(s);
    }
    catch (const Exception& e)
    {
        MO_NETLOG(ERROR, "Failed to deserialize ProjectionSystemSettings\n"
                  << e.what());
    }
}

} // namespace MO
