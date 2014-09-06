/** @file tcpserver.h

    @brief Tcp connection listener

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_NETWORK_TCPSERVER_H
#define MOSRC_NETWORK_TCPSERVER_H

#include <QList>
#include <QObject>
#include <QTcpServer>

class QTcpSocket;

namespace MO {

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

    // --------------- info ----------------

    /** Should return true after a call to open() */
    bool isListening() const;

    /** Returns the number of open connections */
    int openConnections() const { return sockets_.size(); }

    /** Returns human readable name of a socket */
    const QString& socketName(QTcpSocket*) const;

signals:

    /** A new connection was established */
    void socketOpened(QTcpSocket *);
    /** The connection was closed */
    void socketClosed(QTcpSocket *);
    /** There was an error in the connection */
    void socketError(QTcpSocket *);

    /** Data is available for the connection */
    void socketData(QTcpSocket *);
    /** The number of @p bytes was written to the connection */
    void socketDataWritten(QTcpSocket *, qint64 bytes);

public slots:

    /** Starts listening on all adresses on the
        port that is set in the application settings.
        Returns success. */
    bool open();
    /** Stops listening */
    void close();


    // ____________ PRIVATE AREA ______________

private slots:

    void onNewConnection_();
    void onAcceptError_(QAbstractSocket::SocketError);

private:

    QTcpServer * server_;
    QMap<QTcpSocket*, QString> sockets_;
};

} // namespace MO

#endif // MOSRC_NETWORK_TCPSERVER_H
