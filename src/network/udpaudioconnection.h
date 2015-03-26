/** @file udpaudioconnection.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.01.2015</p>
*/

#ifndef MOSRC_NETWORK_UDPAUDIOCONNECTION_H
#define MOSRC_NETWORK_UDPAUDIOCONNECTION_H

#include <QObject>

#include "types/float.h"

namespace MO {
namespace AUDIO { class AudioBuffer; }

class AudioObject;
class UdpConnection;

/** Server and client in one class */
class UdpAudioConnection : public QObject
{
    Q_OBJECT
public:
    explicit UdpAudioConnection(QObject *parent = 0);
    ~UdpAudioConnection();

    bool isOpen() const;

signals:

public slots:

    /** Starts receiving of buffers send from server. */
    bool openForRead();
    void close();

    /** Forgets all buffers */
    void clear();

    /** Makes the buffer known to the server/client.
        If the buffer was added already, false is returned. */
    bool addBuffer(AUDIO::AudioBuffer * buffer, AudioObject * obj, uint channel);

#ifndef MO_DISABLE_SERVER
    /** Broadcasts the current sample block in @p buffer for the given global sample time @p pos.
        @note Buffer needs to be added with addBuffer(). */
    bool sendAudioBuffer(AUDIO::AudioBuffer * buffer, SamplePos pos);
#endif

    /** Adds a connection to the list of receivers */
    //void addConnection(const QString& address, uint16_t port);

private slots:

    void receive_();

private:

    struct Private;
    Private * p_;

};

} // namespace MO

#endif // MOSRC_NETWORK_UDPAUDIOCONNECTION_H
