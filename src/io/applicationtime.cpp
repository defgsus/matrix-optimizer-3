/** @file applicationtime.cpp

    @brief reports time since application start

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/
#include "io/memory.h"

#include <QTime>

#include "applicationtime.h"

namespace MO {

struct AppTime
{
    QTime time;
    AppTime() { time.start(); }
};
static AppTime appTime;

double applicationTime()
{
    return 0.001 * appTime.time.elapsed();
}


QString applicationTimeString()
{
    return QString::number(applicationTime());
}



}
