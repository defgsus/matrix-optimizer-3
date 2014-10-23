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
#include "io/commandlineparser.h"
#include "io/version.h"

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
    cl_         (new IO::CommandLineParser)
{
    initCommandLine_();
}

ClientEngine::~ClientEngine()
{
    delete cl_;
}

void ClientEngine::initCommandLine_()
{
    cl_->addParameter("help", "h,help",
                      tr("Prints the help and exits"));
    cl_->addParameter("info", "info",
                      tr("Prints the system info and exits"));
    cl_->addParameter("infowin", "infowin, infowindow",
                      tr("Displays the info window on the selected desktop"));
    cl_->addParameter("server", "server",
                      tr("Sets the server IP. The client will constantly try to "
                         "connect itself to that address and the ip is stored as the default.\n"
                         "The parameter accepts the common ip notation (xxx.yyy.zzz.www)"),
                      "192.168.1.33");
    cl_->addParameter("desktop", "desktop",
                      tr("Selects the desktop for output windows. "
                         "The value will be stored as default."),
                      0u);
    cl_->addParameter("nonet", "nonet",
                      tr("Turns off networking for the client. "
                         "This is for debugging purposes only."));
    cl_->addParameter("scene", "scene",
                      tr("The client will load and run the specified scene file upon start."),
                      "");
}

bool ClientEngine::parseCommandLine_(int argc, char **argv)
{
#if (1)
    // compile-time commandline params for debug purposes
    Q_UNUSED(argc)
    Q_UNUSED(argv)
    QStringList args;
    args
//            << "-info"
            << "-nonet"
            << "-desktop" << "1"
            //<< "-infowin"
            //<< "-scene" << "../matrixoptimizer3/data/scene/boxgrid2.mo3"
            ;
    if (!cl_->parse(args))
#else

    // check commandline
    if (!cl_->parse(argc, argv, 1))
#endif
    {
        MO_PRINT(tr("Use -h to get help"));
        return false;
    }

    if (cl_->contains("help"))
    {
        MO_PRINT(tr("Usage") << ":\n" << cl_->helpString());
        return false;
    }

    if (cl_->contains("info"))
    {
        SystemInfo info;
        info.get();
        MO_PRINT(tr("Info") << ":\n" << info.toString());
        return false;
    }

    doNetwork_ = !cl_->contains("nonet");
    doShowInfoWin_ = cl_->contains("infowin");

    // set server IP
    if (cl_->contains("server"))
    {
        QString ip = cl_->value("server").toString();
        QUrl url(ip);
        if (!url.isValid())
        {
            MO_PRINT(tr("Could not parse the server ip") << " '" << ip << "'");
            return false;
        }
        settings->setValue("Client/serverAddress", ip);
        MO_PRINT("server ip: " << ip);
    }

    // set desktop
    if (cl_->contains("desktop"))
    {
        const int idx = cl_->value("desktop").toInt();
        settings->setDesktop(idx);
        MO_PRINT(tr("desktop index") << ": " << idx);
    }

    // scene file
    if (cl_->contains("scene"))
    {
        sceneFile_ = cl_->value("scene").toString();
        MO_PRINT(tr("fixed scene file") << ": " << sceneFile_);
    }

    return true;
}







int ClientEngine::run(int argc, char ** argv)
{
    // print some info
    MO_PRINT(applicationName());

    if (!parseCommandLine_(argc, argv))
        return -1;

//    createObjects_();

    // load and run a scene
    if (!sceneFile_.isEmpty())
    {
        if (!loadSceneFile_(sceneFile_))
            return -1;
        showRenderWindow_(true);
        // XXX hack: trigger first clip row
        auto cc = scene_->findChildObjects<ClipContainer>(QString(), true);
        if (!cc.isEmpty())
            cc[0]->triggerRow(0, 0);
        scene_->start();
    }

    if (doNetwork_)
        startNetwork_();

    if (doShowInfoWin_)
        showInfoWindow_(true);


    int ret = 0;

    if (!doNetwork_ && application->allWindows().isEmpty())
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
    client_ = new Client(this);

    connect(client_, SIGNAL(eventReceived(AbstractNetEvent*)), this, SLOT(onNetEvent_(AbstractNetEvent*)));

    client_->connectTo(settings->getValue("Client/serverAddress").toString());
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
}

void ClientEngine::showRenderWindow_(bool enable)
{
    if (enable)
    {
        if (!glWindow_)
            createGlObjects_();

        // move to desired desktop
        glWindow_->setScreen(settings->desktop());
        // show
        glWindow_->showFullScreen();

    }
    else
    {
        if (glWindow_)
            glWindow_->hide();
    }
}

void ClientEngine::renderWindowSizeChanged_(const QSize & size)
{
    if (scene_)
        scene_->setResolution(size);
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
            return;
        }

        if (e->request() == NetEventRequest::GET_CLIENT_INDEX)
        {
            auto r = e->createResponse<NetEventInfo>();
            r->setRequest(e->request());
            r->setData(settings->clientIndex());
            client_->sendEvent(r);
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
        return;
    }

    if (NetEventFile * e = netevent_cast<NetEventFile>(event))
    {
        IO::clientFiles().receiveFile(e);
        return;
    }

    if (NetEventScene * e = netevent_cast<NetEventScene>(event))
    {
        Scene * scene = e->getScene();
        if (scene)
            setSceneObject(scene);
        else
            MO_NETLOG(ERROR, "received invalid Scene object");
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

    glManager_->setScene(scene_);

    // connect to render window
    connect(scene_, SIGNAL(playbackStarted()),
            glWindow_, SLOT(startAnimation()));
    connect(scene_, SIGNAL(playbackStopped()),
            glWindow_, SLOT(stopAnimation()));

    // check for needed files

    IO::FileList files;
    scene_->getNeededFiles(files);

    IO::fileManager().clear();
    IO::fileManager().addFilenames(files);

    IO::fileManager().acquireFiles();

    //glWindow_->renderLater();
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


} // namespace MO
