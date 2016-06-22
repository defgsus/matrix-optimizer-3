/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/8/2016</p>
*/

#ifndef MOSRC_IO_LOG_TEXTURE_H
#define MOSRC_IO_LOG_TEXTURE_H

#include "log.h"

#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_TEX
#   define MO_DEBUG_TEX(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#   define MO_DEBUG_IMG(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_TEX(unused__) { }
#   define MO_DEBUG_IMG(unused__) { }
#endif

#endif // MOSRC_IO_LOG_TEXTURE_H

