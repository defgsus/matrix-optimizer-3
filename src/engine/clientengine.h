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
namespace GUI { class InfoWindow; }

class Client;
class ClientEngine;

/** Returns singleton instance */
ClientEngine & clientEngine();

class ClientEngine : public QObject
{
    Q_OBJECT
public:
    explicit ClientEngine(QObject *parent = 0);

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

private:

    void createGlObjects_();
    void startNetwork_();
    void shutDown_();
    void setProjectionSettings_(NetEventRequest*);

    GL::Manager * glManager_;
    GL::Window * glWindow_;
    GUI::InfoWindow * infoWindow_;

    Client * client_;

    Scene * scene_;
};

} // namespace MO

#endif // MOSRC_ENGINE_CLIENT_H
