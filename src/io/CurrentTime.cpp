/** @file currenttime.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2014</p>
*/

#include "CurrentTime.h"
#include "ApplicationTime.h"
#include "network/NetEvent.h"
#include "engine/ServerEngine.h"
#include "engine/LiveAudioEngine.h"
#include "io/Application.h"
#include "io/isclient.h"
//#include "io/log.h"


namespace MO {

namespace {

    static Double curTime_ = 0.;
    //static Double startTime_ = applicationTime();
    static LiveAudioEngine * audioEngine_ = 0;

}

Double CurrentTime::time()
{
#ifndef MO_DISABLE_CLIENT
    if (audioEngine_)
        return audioEngine_->second();
#endif

    return curTime_;//applicationTime() - startTime_;
}

void CurrentTime::setTime(Double time)
{
    //MO_DEBUG("CurrentTime::setSceneTime(" << time << ")");

    //startTime_ = applicationTime() - time;
    curTime_ = time;

#ifndef MO_DISABLE_SERVER
    if (isServer() && serverEngine().isRunning())
    {
        auto e = new NetEventTime();
        e->setTime(time);
        serverEngine().sendEvent(e);
    }
#endif
}

void CurrentTime::setAudioEngine(LiveAudioEngine * ae)
{
    if (!audioEngine_)
        ae->seek(time());
    audioEngine_ = ae;
}


}
