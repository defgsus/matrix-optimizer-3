/** @file log.h

    @brief debug output

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSRC_IO_LOG_H
#define MOSRC_IO_LOG_H

#include <QString>
#include "io/lockedoutput.h"
#include "io/console.h"
#include "io/streamoperators_qt.h"
#include "io/applicationtime.h"
#include "io/currentthread.h"


#ifndef NDEBUG
/** Enable debug output (MO_DEBUG...()) */
#   define MO_ENABLE_DEBUG
#endif

#define MO_PRINT(stream_arg__) \
    MO_STREAM_OUT(std::cout, stream_arg__ << std::endl)

#ifdef MO_ENABLE_DEBUG
    #define MO_DEBUG_IMPL_(stream_arg__)                    \
        MO_STREAM_OUT(std::cerr,                            \
               "[" << ::MO::currentThreadName()             \
            << "/" << ::MO::applicationTimeString() << "] " \
            << ::MO::streamColor::Debug << stream_arg__     \
            << ::MO::streamColor::Default << std::endl)
#else
    #define MO_DEBUG_IMPL_(unused__) { }
#endif


#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG
#   define MO_DEBUG(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUGF
#   define MO_DEBUGF(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUGF(unused__) { }
#endif




#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_RENDER
#   define MO_DEBUG_RENDER(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_RENDER(unused__) { }
#endif






#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_HELP
#   define MO_DEBUG_HELP(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_HELP(unused__) { }
#endif



#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_CLIP
#   define MO_DEBUG_CLIP(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_CLIP(unused__) { }
#endif

#endif // MOSRC_IO_LOG_H
