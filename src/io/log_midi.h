/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/8/2016</p>
*/

#ifndef MOSRC_IO_LOG_MIDI_H
#define MOSRC_IO_LOG_MIDI_H

#include "log.h"

#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_MIDI
#   define MO_DEBUG_MIDI(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_MIDI(unused__) { }
#endif

#endif // MOSRC_IO_LOG_MIDI_H
