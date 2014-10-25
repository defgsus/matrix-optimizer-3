/** @file serverengine.h

    @brief Server/client responsibility wraper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#ifndef MOSRC_ENGINE_SERVERENGINE_H
#define MOSRC_ENGINE_SERVERENGINE_H

#include <QObject>

#include "network/network_fwd.h"
#include "io/systeminfo.h"
#include "network/clientstate.h"
#include "projection/projectionsystemsettings.h"

class QTcpSocket;

namespace MO {

class ServerEngine;
class Scene;

/** Returns a singleton instance of the server */
ServerEngine & serverEngine();

class ClientInfo
{
public:

    /** Connection to/from each client */
    QTcpSocket * tcpSocket;

    /** Index of the client (as in ProjectionSystemSettings) */
    int index;

    /** System-info of each client */
    SystemInfo sysinfo;

    /** Current state of each client */
    ClientState state;

    struct Private;
    Private * p_;
};


class ServerEngine : public QObject
{
    Q_OBJECT
public:

    // -------------- ctor ---------------------

    explicit ServerEngine(QObject *parent = 0);
    ~ServerEngine();

    // -------------- getter -------------------

    bool isRunning() const;

    int numClients() const;

    const ClientInfo& clientInfo(int index) const { return clients_[index]; }

    /** Returns the one tcp server */
    TcpServer * tcpServer() const { return server_; }

    /** Creates a new projection set from the info of the
        connected clients.
        @note If no client is connected, the settings will not
        contain anything. */
    ProjectionSystemSettings createProjectionSystemSettings();

    // ------------ network --------------------

    bool open();
    void close();

    // -------------- events -------------------

    /** Sends the event to all connected clients.
        Ownership is taken. */
    bool sendEvent(AbstractNetEvent*);

    /** Sends the event to the particular client.
        Ownership is taken. */
    bool sendEvent(ClientInfo&, AbstractNetEvent*);

signals:

    /** Emitted whenever a client connects or disconnects */
    void numberClientsChanged(int);

    /** Emitted when a client has send a new ClientState */
    void clientStateChanged(int index);

public slots:

    // -- per client commands --

    void showInfoWindow(int index, bool show);
    void showRenderWindow(int index, bool enable);

    void setClientIndex(int index, int client_index);
    void setDesktopIndex(int index, int desktopIndex);

    // -- commands for all clients --

    /** Sends the current default ProjectionSystemSettings to all clients */
    void sendProjectionSettings();

    /** Send the scene to all clients */
    bool sendScene(Scene * scene);

    /** Start and stop playback */
    void setScenePlaying(bool enabled);

private slots:

    void onTcpConnected_(QTcpSocket*);
    void onTcpDisconnected_(QTcpSocket*);
    void onTcpError_(QTcpSocket*);
    void onTcpData_(QTcpSocket*);
    void onEventCom_(AbstractNetEvent*);
    void onEvent_(ClientInfo& , AbstractNetEvent*);

private:

    /** Returns the index in clients_, or -1 */
    int clientForTcp_(QTcpSocket*);

    void getSysInfo_(ClientInfo&);
    void getClientIndex_(ClientInfo&);
    void sendProjectionSettings_(ClientInfo&);

    QList<ClientInfo> clients_;

    TcpServer * server_;
    EventCom * eventCom_;
};

} // namespace MO


#endif // MOSRC_ENGINE_SERVERENGINE_H
