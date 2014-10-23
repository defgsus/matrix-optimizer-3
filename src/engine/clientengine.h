/** @file client.h

    @brief Client runtime process

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_ENGINE_CLIENT_H
#define MOSRC_ENGINE_CLIENT_H

#include <QObject>

#include "gl/opengl_fwd.h"
#include "network/network_fwd.h"
#include "io/filetypes.h"

namespace MO {
class Scene;
class ClientEngineCommandLine;
namespace GUI { class InfoWindow; }

class Client;
class ClientEngine;

/** Returns singleton instance */
ClientEngine & clientEngine();


/**
    The client engine basically creates an GL::Manager and GL::Window,
    a network Client and connects all the signals.

 */
class ClientEngine : public QObject
{
    Q_OBJECT
public:
    explicit ClientEngine(QObject *parent = 0);
    ~ClientEngine();

    /** Runs the complete client event loop */
    int run(int argc, char ** argv);

signals:

public slots:

    /** Sends event to server.
        Ownership of event is taken. */
    bool sendEvent(AbstractNetEvent *);

    /** Sets the current Scene. Ownership is taken. */
    void setSceneObject(Scene *);

private slots:

    void onNetEvent_(AbstractNetEvent *);
    void showInfoWindow_(bool enable);
    void showRenderWindow_(bool enable);
    void renderWindowSizeChanged_(const QSize&);

    void onSceneReceived_(Scene *);
    void onFilesReady_();
    void onFilesNotReady_();
    /** Sends a NetEventClientState to server */
    void sendState_();
private:

    void createGlObjects_();
    void updateDesktopIndex_();
    void startNetwork_();
    void shutDown_();
    void setProjectionSettings_(NetEventRequest*);
    bool loadSceneFile_(const QString& fn);
    void setPlayback_(bool play);

    GL::Manager * glManager_;
    GL::Window * glWindow_;
    GUI::InfoWindow * infoWindow_;

    Client * client_;

    Scene * scene_, * nextScene_;

    ClientEngineCommandLine * cl_;
};

} // namespace MO

#endif // MOSRC_ENGINE_CLIENT_H
