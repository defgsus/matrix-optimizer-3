/** @file clientstate.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#ifndef MOSRC_NETWORK_CLIENTSTATE_H
#define MOSRC_NETWORK_CLIENTSTATE_H

namespace MO {
namespace IO { class DataStream; }

/** Current state of a client */
class ClientState
{
public:

    void serialize(IO::DataStream& io) const;
    void deserialize(IO::DataStream& io);

    // --------- getter -------------------

    /** Projector index */
    int clientIndex() const { return index_; }
    /** Output desktop/screen */
    int desktop() const { return desktop_; }

    // state
    bool isPlayback() const { return isPlayback_; }
    bool isInfoWindow() const { return isInfoWindow_; }
    bool isRenderWindow() const { return isRenderWindow_; }
    bool isSceneReady() const { return isSceneReady_; }

private:

    friend class ClientEngine;

    bool isPlayback_, isInfoWindow_, isRenderWindow_, isSceneReady_;
    int index_, desktop_;
};


} // namespace MO

#endif // MOSRC_NETWORK_CLIENTSTATE_H
