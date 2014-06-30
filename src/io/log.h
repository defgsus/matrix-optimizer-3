/** @file log.h

    @brief debug output

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSRC_IO_LOG_H
#define MOSRC_IO_LOG_H

#include <QString>
#include "io/console.h"
#include "io/streamoperators_qt.h"
#include "io/applicationtime.h"


#ifndef NDEBUG
/** Enable debug output (MO_DEBUG...()) */
#   define MO_ENABLE_DEBUG
#endif


#ifdef MO_ENABLE_DEBUG
    #define MO_DEBUG_IMPL_(stream_arg__) \
        { std::cerr << "[" << ::MO::applicationTimeString() << "] " \
            << streamColor::Debug << stream_arg__ << streamColor::Default << std::endl; }
#else
    #define MO_DEBUG_IMPL_(unused__) { }
#endif


#if (1)
#   define MO_DEBUG(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG(unused__) { }
#endif

#if (0)
#   define MO_DEBUGF(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUGF(unused__) { }
#endif

#if (0)
#   define MO_DEBUG_IO(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_IO(unused__) { }
#endif

#if (0)
#   define MO_DEBUG_GL(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_GL(unused__) { }
#endif

#if (0)
#   define MO_DEBUG_TREE(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_TREE(unused__) { }
#endif



#endif // MOSRC_IO_LOG_H
