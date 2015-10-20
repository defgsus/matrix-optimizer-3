/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#ifndef MOSRC_NETWORK_OSCINPUT_H
#define MOSRC_NETWORK_OSCINPUT_H

#include <QObject>
#include <QList>
#include <QHostAddress>

#include "types/refcounted.h"

namespace MO {


/** Open Sound Control input module */
class OscInput : public QObject, public RefCounted
{
    Q_OBJECT
    ~OscInput();
    struct Private;
    Private * p_;

public:
    OscInput(QObject * parent = 0);

    // -------------- network io ---------------

    /** Starts listening on @p port for incomming datagrams from @p addr */
    bool open(const QHostAddress& addr, uint16_t port);

    /** Starts listening on @p port for any incomming datagrams */
    bool open(uint16_t port);

    /** Closes the existing connection */
    void close();

    /** Returns the assigned port number */
    uint16_t port() const;

    // -------------- values -------------------

    /** Returns the value for @p id, or an invalid QVariant */
    const QVariant& value(const QString& id) const;
    /** Returns true if the value for @p id has been received yet */
    bool hasValue(const QString& id) const;

    /** Read-access to all values yet received */
    const QMap<QString, QVariant>& values() const;

signals:

    /** Emitted when a value has been received */
    void valueChanged(const QString& id);

    /** Emitted when getListener() created a listener or
        releaseListener() removed a listener */
    void listenersChanged();
};


} // namespace MO

#endif // MOSRC_NETWORK_OSCINPUT_H
