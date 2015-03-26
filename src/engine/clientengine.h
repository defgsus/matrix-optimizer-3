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
#include "types/float.h"
#include "audio/configuration.h"

namespace MO {
namespace GUI { class InfoWindow; }

class Scene;
class Client;
class ClientEngine;
class ClientEngineCommandLine;
class AudioEngine;


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

    bool isRunning() const;

    /** Runs the complete client event loop */
    int run(int argc, char ** argv, int skip);

    Double curTime() const;

signals:

public slots:

    /** Sends event to server.
        Ownership of event is taken. */
    bool sendEvent(AbstractNetEvent *);

    /** Sets the current Scene. Ownership is taken. */
    void setSceneObject(Scene *);

    void setAudioConfig(const AUDIO::Configuration&);

private slots:

    void onConnected_();
    void onNetEvent_(AbstractNetEvent *);
    void showInfoWindow_(bool enable);
    void showRenderWindow_(bool enable);
    void renderWindowSizeChanged_(const QSize&);
    void onNetLog_(int level, const QString& text);

    void onSceneReceived_(Scene *);
    void onFilesReady_();
    void onFilesNotReady_();
    /** Sends a NetEventClientState to server */
    void sendState_();

    /** Renders the scene if not running */
    void render_();


private:

    void createGlObjects_();
    void updateDesktopIndex_();
    void startNetwork_();
    void shutDown_();
    void setProjectionSettings_(NetEventRequest*);
    bool loadSceneFile_(const QString& fn);
    void setPlayback_(bool play);
    void setClientIndex_(int index);
    void setNextScene_();

    GL::Manager * glManager_;
    GL::Window * glWindow_;
    GUI::InfoWindow * infoWindow_;

    Client * client_;

    Scene * scene_, * nextScene_;
    AudioEngine * audioEngine_;
    AUDIO::Configuration audioConfig_;

    ClientEngineCommandLine * cl_;

    bool isFilesReady_;
};

} // namespace MO

#endif // MOSRC_ENGINE_CLIENT_H
