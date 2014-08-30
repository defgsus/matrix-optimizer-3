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

    /** Returns a list of all networks available */
    QString systemInfo() const;

    /** Returns list of ips available */
    QString networkInfo() const;

    // ---------- initialization -----------

    /** Returns the set configuration (ethernet & active),
        or 0 if not found. */
    QNetworkConfiguration * defaultNetwork() const;

    bool isOpen() const;

signals:

    void error(const QString& text);
    void opened();
    void closed();

public slots:

    /** Tries to open the default network.
        The opened() signal will be emitted on success. */
    void open();

    /** Tries to close the default network.
        The closed() signal will be emitted on success. */
    void close();

protected slots:

    void slotOpened_();
    void slotClosed_();
    void slotError_(QNetworkSession::SessionError);

protected:

    QNetworkConfigurationManager * conf_;
    //QNetworkAccessManager * mgr_;
    QNetworkSession * net_;
};

} // namespace MO


#endif // NETWORKMANAGER_H
