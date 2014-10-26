/** @file clientstate.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.10.2014</p>
*/

#include "clientstate.h"
#include "io/datastream.h"

namespace MO {

void ClientState::serialize(IO::DataStream &io) const
{
    io << index_ << desktop_
       << isPlayback_ << isInfoWindow_ << isRenderWindow_
       << isSceneReady_ << isFilesReady_;
}

void ClientState::deserialize(IO::DataStream &io)
{
    io >> index_ >> desktop_
       >> isPlayback_ >> isInfoWindow_ >> isRenderWindow_
       >> isSceneReady_ >> isFilesReady_;
}


} // namespace MO

