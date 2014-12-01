/** @file time.h

    @brief XXX Would like to have a precise sample/float-second unit at the bottom

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef MOSRC_TYPES_TIME_H
#define MOSRC_TYPES_TIME_H

#include <cstdint>

namespace MO
{

    struct Time_t
    {
        int32_t second, sample, sampleRate;
    };



} // namespace MO

#endif // MOSRC_TYPES_TIME_H
