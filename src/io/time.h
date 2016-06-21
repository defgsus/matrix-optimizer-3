/** @file io/time.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 20.02.2015</p>
*/

#ifndef MOSRC_IO_TIME_H
#define MOSRC_IO_TIME_H

#include "types/float.h"

namespace MO {

void testClock();

/** High resolution current system-up time in seconds */
double systemTime();

/** Simple class to messure time.
    Constructor and start() store the current time.
    time() returns the seconds passed since creation or start().
    */
class TimeMessure
{
public:
    /** default constructor stores current time */
    TimeMessure() { start(); }

    /** (re-)stores current time */
    void start() { stime_ = systemTime(); }

    double time() { return systemTime() - stime_; }

private:
    /** start time */
    double stime_;
    /** disable copy */ TimeMessure(const TimeMessure&);
    /** disable copy */ TimeMessure& operator=(const TimeMessure&);
};


/** High-res sleep */
void sleep_seconds(double sec);

/** Common low-res sleep (~16ms on Win) */
void sleep_seconds_lowres(double sec);

} // namespace MO

#endif // MOSRC_IO_TIME_H
