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

class QTcpSocket;

namespace MO {

struct ClientInfo
{
    QTcpSocket * tcpSocket;

    int index;

    SystemInfo sysinfo;

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

    int numOpenConnections() const;

    /** Returns the one tcp server */
    TcpServer * tcpServer() const { return server_; }

    // ------------ network --------------------

    bool open();
    void close();

    // -------------- events -------------------

    /** Sends the event to all connected clients.
        Ownership is taken. */
    void sendEvent(AbstractNetEvent*);

signals:

    /** Emitted whenever a client connects or disconnects */
    void numberClientsChanged(int);

public slots:

private slots:

    void onTcpConnected_(QTcpSocket*);
    void onTcpDisonnected_(QTcpSocket*);
    void onTcpError_(QTcpSocket*);
    void onTcpData_(QTcpSocket*);

private:

    /** Returns the index in clients_, or -1 */
    int clientForTcp_(QTcpSocket*);

    void getSysInfo_(ClientInfo&);
    void getClientIndex_(ClientInfo&);

    QList<ClientInfo> clients_;

    TcpServer * server_;

};

} // namespace MO


#endif // MOSRC_ENGINE_SERVERENGINE_H
