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
#include "io/clientfiles.h"

namespace MO {

ClientEngine & clientEngine()
{
    static ClientEngine * instance_ = 0;
    if (!instance_)
        instance_ = new ClientEngine(application);

    return *instance_;
}

ClientEngine::ClientEngine(QObject *parent) :
    QObject     (parent),
    glManager_  (0),
    glWindow_   (0),
    infoWindow_ (0),
    client_     (0),
    scene_      (0)
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

bool ClientEngine::sendEvent(AbstractNetEvent * event)
{
    return client_->sendEvent(event);
}

void ClientEngine::createGlObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);
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

void ClientEngine::showRenderWindow_(bool enable)
{
    if (enable)
    {
        if (!glWindow_)
            createGlObjects_();

        glWindow_->showFullScreen();
    }
    else
    {
        if (glWindow_)
            glWindow_->hide();
    }
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

    if (NetEventFileInfo * e = netevent_cast<NetEventFileInfo>(event))
    {
        IO::clientFiles().receiveFileInfo(e);
    }

    if (NetEventFile * e = netevent_cast<NetEventFile>(event))
    {
        IO::clientFiles().receiveFile(e);
    }

    if (NetEventScene * e = netevent_cast<NetEventScene>(event))
    {
        Scene * scene = e->getScene();
        if (scene)
            setSceneObject(scene);
        else
            MO_NETLOG(ERROR, "received invalid Scene object");
    }

    MO_NETLOG(WARNING, "unhandled NetEvent " << event->className() << " in ClientEngine");

    delete event;
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

void ClientEngine::setSceneObject(Scene * scene)
{
    // create gl objects

    if (!glManager_ || !glWindow_)
        createGlObjects_();

    // delete previous scene
    if (scene_)
    {
        scene_->kill();
        scene_->deleteLater();
    }

    scene_ = scene;

    // manage memory
    scene_->setParent(this);

    // connect to render window
    connect(glManager_, SIGNAL(renderRequest(uint)), scene_, SLOT(renderScene(uint)));
    connect(glManager_, SIGNAL(contextCreated(uint,MO::GL::Context*)),
                scene_, SLOT(setGlContext(uint,MO::GL::Context*)));
    connect(glManager_, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                scene_, SLOT(setFreeCameraMatrix(MO::Mat4)));

    connect(scene_, SIGNAL(renderRequest()), glWindow_, SLOT(renderLater()));

    if (glWindow_->context())
        scene_->setGlContext(glWindow_->threadId(), glWindow_->context());
    connect(scene_, SIGNAL(playbackStarted()),
            glWindow_, SLOT(startAnimation()));
    connect(scene_, SIGNAL(playbackStopped()),
            glWindow_, SLOT(stopAnimation()));

    glWindow_->renderLater();
}


} // namespace MO
