/** @file currenttime.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2014</p>
*/

#include "currenttime.h"
#include "applicationtime.h"
#include "network/netevent.h"
#include "engine/serverengine.h"
#include "engine/liveaudioengine.h"
#include "io/application.h"
#include "io/isclient.h"
//#include "io/log.h"


namespace MO {

namespace {


static Double startTime_ = applicationTime();
static LiveAudioEngine * audioEngine_ = 0;

}

Double CurrentTime::time()
{
#ifndef MO_DISABLE_CLIENT
    if (audioEngine_)
        return audioEngine_->second();
#endif

    return applicationTime() - startTime_;
}

void CurrentTime::setTime(Double time)
{
    //MO_DEBUG("CurrentTime::setSceneTime(" << time << ")");

    startTime_ = applicationTime() - time;

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
    audioEngine_ = ae;
}


}
