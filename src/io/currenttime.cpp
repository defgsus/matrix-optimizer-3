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
#include "io/application.h"
#include "io/isclient.h"

namespace MO {

Double CurrentTime::startTime_ = applicationTime();

Double CurrentTime::time()
{
    return applicationTime() - startTime_;
}

void CurrentTime::setTime(Double time)
{
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

}
