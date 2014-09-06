/** @file networkmanager.h

    @brief Basic wrapper around Qt's network stuff

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkSession>

class QNetworkConfiguration;
class QNetworkConfigurationManager;
class QNetworkAccessManager;

namespace MO {

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = 0);

    // ---------- info ---------------------

    /** Returns whether the application settings are configured
        for networking */
    bool isConfigured() const;

    /** Returns the network name from settings */
    QString defaultNetworkName() const;

    /** Returns the udp port from settings */
    int defaultUdpPort() const;

    /** Returns the udp port from settings */
    int defaultTcpPort() const;

    /** Returns a list of all networks available */
    QString systemInfo() const;

    /** Returns list of ips available */
    QString networkInfo() const;

    // ---------- initialization -----------

    /** Returns the configuration from settings that
        has ethernet & active flags.
        If settings are not configured, the first active ethernet is returned.
        Returns NULL if nothing was found. */
    QNetworkConfiguration * defaultNetwork() const;

    /** Returns true when a network connection has been
        successfully opened with open(). */
    bool isOpen() const;

    /** Returns the current session object (when opened), or NULL.
        Ownership stays with this class. */
    QNetworkSession * currentSession() const;

signals:

    /** Emitted on network errors */
    void error(const QString& text);
    /** Emitted on succesful open() */
    void opened();
    /** Emitted on succesful close() or when the network goes down. */
    void closed();

public slots:

    /** Tries to open the default network.
        The opened() signal will be emitted on success. */
    void open();

    /** Tries to close the currently open network.
        The closed() signal will be emitted on success. */
    void close();

protected slots:

    void slotOpened_();
    void slotClosed_();
    void slotError_(QNetworkSession::SessionError);

protected:

    QNetworkConfigurationManager * conf_;
    QNetworkSession * net_;
};

} // namespace MO


#endif // NETWORKMANAGER_H
