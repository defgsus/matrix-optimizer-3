/** @file applicationtime.cpp

    @brief reports time since application start

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "applicationtime.h"
#include "time.h"

namespace MO {

namespace {

    static TimeMessure appTime;

} // namespace

double applicationTime()
{
    return appTime.time();
}


QString applicationTimeString()
{
    return QString::number(applicationTime());
}

}
