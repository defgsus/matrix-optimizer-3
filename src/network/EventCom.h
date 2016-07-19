/** @file eventcom.h

    @brief NetEvent sender/receiver

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/22/2014</p>
*/

#ifndef MOSRC_NETWORK_EVENTCOM_H
#define MOSRC_NETWORK_EVENTCOM_H

#include <QObject>
#include <QByteArray>

class QAbstractSocket;

namespace MO {

class AbstractNetEvent;

class EventCom : public QObject
{
    Q_OBJECT
public:
    explicit EventCom(QObject *parent = 0);

signals:

    /** Emitted on received events.
        @warning The ownership is transfered to the receiver,
        so there MUST be a receiver to delete the events! */
    void eventReceived(AbstractNetEvent *);

public slots:

    /** Pushes data to the internal buffer.
        Whenever a full event-packet is received, eventReceived() is emitted */
    void inputData(QAbstractSocket *);

    /** Sends the event to the socket, ownership stays will caller */
    bool sendEvent(QAbstractSocket *, AbstractNetEvent *);

private:

    void processPacket_(const QByteArray&, QAbstractSocket * sender);

    qint64 bufferSize_;
    QByteArray buffer_;
};

} // namespace MO

#endif // MOSRC_NETWORK_EVENTCOM_H
