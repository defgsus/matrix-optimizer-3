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


namespace MO {

class ServerEngine : public QObject
{
    Q_OBJECT
public:
    explicit ServerEngine(QObject *parent = 0);

    // -------------- getter -------------------

    bool isRunning() const;

    /** Returns the one tcp server */
    TcpServer * tcpServer() const { return server_; }

    // ------------ network --------------------

    bool open();
    void close();

signals:

public slots:

private:

    TcpServer * server_;

};

} // namespace MO


#endif // MOSRC_ENGINE_SERVERENGINE_H
