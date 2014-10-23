/** @file currenttime.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/23/2014</p>
*/

#ifndef MOSRC_IO_CURRENTTIME_H
#define MOSRC_IO_CURRENTTIME_H

#include <types/float.h>

namespace MO {

/** Should return the current project time according to timecode or some other
    global counter */
Double currentTime();


} // namespace MO

#endif // MOSRC_IO_CURRENTTIME_H
