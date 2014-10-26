/** @file client.cpp

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTcpSocket>
#include <QUrl>

#include "clientengine.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "object/clipcontainer.h"
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
#include "io/filemanager.h"
#include "tool/deleter.h"
#include "io/version.h"
#include "clientenginecommandline.h"
#include "io/currenttime.h"

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
    scene_      (0),
    nextScene_  (0),
    cl_         (new ClientEngineCommandLine(this))
{
    MO_NETLOG(CTOR, "ClientEngine::ClientEngine()");

    connect(&NetworkLogger::instance(), SIGNAL(textAdded(int, QString)),
            this, SLOT(onNetLog_(int, QString)));
}

ClientEngine::~ClientEngine()
{
    MO_NETLOG(CTOR, "ClientEngine::~ClientEngine()");

    delete nextScene_;
}



bool ClientEngine::isRunning() const
{
    return client_ && client_->isRunning();
}

int ClientEngine::run(int argc, char ** argv)
{
    // print some info
    MO_PRINT(applicationName());

    // parse commandline
    const auto res = cl_->parse(argc, argv);

    if (res == ClientEngineCommandLine::Error)
        return -1;

    if (res == ClientEngineCommandLine::Quit)
        return 0;


    // start network

    if (cl_->doNetwork())
        startNetwork_();

    // show the info window

    if (cl_->doShowInfoWin())
        showInfoWindow_(true);

    // load and run a scene

    if (cl_->doLoadScene())
    {
        if (!loadSceneFile_(cl_->sceneFile()))
            return -1;

        showRenderWindow_(true);
        setPlayback_(true);
    }


    // -- run eventloop --

    int ret = 0;

    // when not connecting to server and no window is opened
    // we exit
    if (!cl_->doNetwork() && application->allWindows().isEmpty())
    {
        MO_PRINT(tr("Nothing to do, exiting..."));
    }
    else

        ret = application->exec();

    shutDown_();

    return ret;
}




void ClientEngine::createGlObjects_()
{
    // manager
    glManager_ = new GL::Manager(this);
    connect(glManager_, SIGNAL(outputSizeChanged(QSize)),
            this, SLOT(renderWindowSizeChanged_(QSize)));

    // window
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);
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


void ClientEngine::startNetwork_()
{
    // create client connection and receive events

    client_ = new Client(this);

    connect(client_, SIGNAL(connected()), this, SLOT(onConnected_()));
    connect(client_, SIGNAL(eventReceived(AbstractNetEvent*)), this, SLOT(onNetEvent_(AbstractNetEvent*)));

    client_->connectTo(settings->serverAddress());

    // receive files-ready signal from file cacher
    connect(&IO::fileManager(), SIGNAL(filesReady()),
            this, SLOT(onFilesReady_()));

    connect(&IO::fileManager(), SIGNAL(finished()),
            this, SLOT(onFilesNotReady_()));
}

void ClientEngine::onNetLog_(int level, const QString &text)
{
    if (!isRunning())
        return;

    // send special log levels back to server
    if (level & (NetworkLogger::APP_WARNING |
                 NetworkLogger::APP_ERROR))
    {
        auto e = new NetEventLog();
        e->setMessage((NetworkLogger::Level)level, text);
        sendEvent(e);
    }
}

void ClientEngine::showInfoWindow_(bool enable)
{
    if (enable)
    {
        if (!infoWindow_)
            infoWindow_ = new GUI::InfoWindow();

        infoWindow_->updateInfo();

        // move to desired desktop
        infoWindow_->setGeometry(
                    application->screenGeometry(
                        settings->desktop()));
        // show
        infoWindow_->showFullScreen();
    }
    else if (infoWindow_)
            infoWindow_->hide();

    sendState_();
}

void ClientEngine::showRenderWindow_(bool enable)
{
    if (enable)
    {
        if (scene_)
        {
            if (!glWindow_)
                createGlObjects_();

            // move to desired desktop
            glWindow_->setScreen(settings->desktop());
            // show
            glWindow_->showFullScreen();
        }
    }
    else
    {
        if (glWindow_)
            glWindow_->hide();
    }

    sendState_();
}

void ClientEngine::updateDesktopIndex_()
{
    if (glWindow_)
    {
        bool vis = glWindow_->isVisible();

        if (vis)
            glWindow_->hide();

        glWindow_->setScreen(settings->desktop());

        // XXX moving by above 'workaround' leaves fullscreen
        if (vis)
            glWindow_->showFullScreen();
    }

    if (infoWindow_)
    {
        bool vis = infoWindow_->isVisible();

        if (vis)
            infoWindow_->hide();

        infoWindow_->setGeometry(application->screenGeometry(
                                     settings->desktop()));
        if (vis)
            infoWindow_->showFullScreen();
    }

    sendState_();
}

void ClientEngine::renderWindowSizeChanged_(const QSize & size)
{
    if (scene_)
        scene_->setResolution(size);
}

void ClientEngine::onConnected_()
{
    MO_NETLOG(DEBUG, "ClientEngine connected :D");
    IO::clientFiles().update();
}

void ClientEngine::onNetEvent_(AbstractNetEvent * event)
{
    ScopedDeleter<AbstractNetEvent> deleter(event);

    if (NetEventRequest * e = netevent_cast<NetEventRequest>(event))
    {
        // respond with system information
        if (e->request() == NetEventRequest::GET_SYSTEM_INFO)
        {
            auto r = e->createResponse<NetEventSysInfo>();
            r->getInfo();
            client_->sendEvent(r);
            // also send state
            sendState_();
            return;
        }

        if (e->request() == NetEventRequest::SET_CLIENT_INDEX)
        {
            MO_NETLOG(EVENT, "setting client index to " << e->data().toInt());
            setClientIndex_(e->data().toInt());
            return;
        }

        if (e->request() == NetEventRequest::SET_DESKTOP_INDEX)
        {
            MO_NETLOG(EVENT, "setting desktop/screen index to " << e->data().toInt());
            settings->setDesktop(e->data().toInt());
            updateDesktopIndex_();
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

        if (e->request() == NetEventRequest::SHOW_RENDER_WINDOW)
        {
            showRenderWindow_(true);
            return;
        }

        if (e->request() == NetEventRequest::HIDE_RENDER_WINDOW)
        {
            showRenderWindow_(false);
            return;
        }

        if (e->request() == NetEventRequest::SET_PROJECTION_SETTINGS)
        {
            setProjectionSettings_(e);
            return;
        }

        if (e->request() == NetEventRequest::START_RENDER)
        {
            setPlayback_(true);
            return;
        }

        if (e->request() == NetEventRequest::STOP_RENDER)
        {
            setPlayback_(false);
            return;
        }
    }

    if (NetEventFileInfo * e = netevent_cast<NetEventFileInfo>(event))
    {
        IO::clientFiles().receiveFileInfo(e);
        return;
    }

    if (NetEventFile * e = netevent_cast<NetEventFile>(event))
    {
        IO::clientFiles().receiveFile(e);
        return;
    }

    if (NetEventTime * e = netevent_cast<NetEventTime>(event))
    {
        CurrentTime::setTime(e->time());
        render_();
        return;
    }

    if (NetEventScene * e = netevent_cast<NetEventScene>(event))
    {
        Scene * scene = e->getScene();
        if (scene)
            onSceneReceived_(scene);
        else
            MO_NETLOG(ERROR, "received invalid Scene object");
        // XXX Need to reflect this in ClientState
        return;
    }

    MO_NETLOG(WARNING, "unhandled NetEvent " << event->infoName() << " in ClientEngine");
}

void ClientEngine::setProjectionSettings_(NetEventRequest * e)
{
    QByteArray data = e->data().toByteArray();

    ProjectionSystemSettings s;

    try
    {
        s.deserialize(data);
        // update system settings
        settings->setDefaultProjectionSettings(s);

        // update scene
        if (scene_)
        {
            scene_->setProjectionSettings(s);
            scene_->setProjectorIndex(settings->clientIndex());
        }
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

    glManager_->setScene(scene_);

    // connect to render window
    connect(scene_, SIGNAL(playbackStarted()),
            glWindow_, SLOT(startAnimation()));
    connect(scene_, SIGNAL(playbackStopped()),
            glWindow_, SLOT(stopAnimation()));

    // update resolution from output window
    scene_->setResolution(glWindow_->frameSize());
    // update projection settings
    scene_->setProjectionSettings(settings->getDefaultProjectionSettings());
    scene_->setProjectorIndex(settings->clientIndex());
}


bool ClientEngine::loadSceneFile_(const QString &fn)
{
    try
    {
        Scene * scene = ObjectFactory::loadScene(fn);
        if (!scene)
        {
            MO_PRINT(tr("No Scene file")<<": " << fn);
            return false;
        }
        setSceneObject(scene);
        return true;
    }
    catch (const Exception& e)
    {
        MO_PRINT(tr("Could not load Scene file") << " " << fn << "\n"
                 << e.what());
    }
    return false;
}

void ClientEngine::setPlayback_(bool play)
{
    if (!scene_)
        return;

    if (play)
    {
        // XXX hack: trigger first clip row
        auto cc = scene_->findChildObjects<ClipContainer>(QString(), true);
        if (!cc.isEmpty())
            cc[0]->triggerRow(0, 0);

        scene_->start();
    }
    else
        scene_->stop();

    sendState_();
}

void ClientEngine::setClientIndex_(int index)
{
    // XXX this variable requires parsing the whole xml
    const int maxi = settings->getDefaultProjectionSettings().numProjectors();

    MO_ASSERT(maxi > 0, "Somethings wrong with the DefaultProjectionSettings");

    if (index >= maxi)
    {
        MO_WARNING("Could not set projector/client index " << index
                   << " because it's out of range (0-" << (maxi-1) << ")");
        index = maxi - 1;
    }
    settings->setClientIndex(index);
    if (scene_)
        scene_->setProjectorIndex(index);
    sendState_();
}

void ClientEngine::onSceneReceived_(Scene * scene)
{
    if (nextScene_)
        delete nextScene_;

    // put on stack
    nextScene_ = scene;

    sendState_();

    // check for needed files

    IO::FileList files;
    nextScene_->getNeededFiles(files);

    IO::fileManager().clear();
    IO::fileManager().addFilenames(files);

    if (!files.isEmpty())
        MO_PRINT("Checking file cache..");

    IO::fileManager().acquireFiles();
}

void ClientEngine::onFilesReady_()
{
    if (!nextScene_)
        return;

    Scene * s = nextScene_;
    nextScene_ = 0;

    setSceneObject(s);

    if (glWindow_ && !glWindow_->isVisible())
        showRenderWindow_(true);

    sendState_();
}

void ClientEngine::onFilesNotReady_()
{
    MO_WARNING("Some file are not ready");

    // run scene nevertheless
    onFilesReady_();
}

void ClientEngine::sendState_()
{
    NetEventClientState state;
    state.state_.index_ = settings->clientIndex();
    state.state_.desktop_ = settings->desktop();
    state.state_.isInfoWindow_ = infoWindow_ && infoWindow_->isVisible();
    state.state_.isRenderWindow_ = glWindow_ && glWindow_->isVisible();
    state.state_.isSceneReady_ = scene_ && !nextScene_;
    state.state_.isPlayback_ = scene_ && scene_->isPlayback();

    client_->sendEvent(state);
}

void ClientEngine::render_()
{
    if (glWindow_ && scene_ && !scene_->isPlayback())
        glWindow_->renderLater();
}

} // namespace MO
