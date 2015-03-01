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

/** High resolution current time in seconds */
Double systemTime();

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

} // namespace MO

#endif // MOSRC_IO_TIME_H
