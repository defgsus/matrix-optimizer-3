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

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_IO
#   define MO_DEBUG_IO(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_IO(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_GL
#   define MO_DEBUG_GL(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_GL(unused__) { }
#endif

// tree changes and updates
#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_TREE
#   define MO_DEBUG_TREE(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_TREE(unused__) { }
#endif

// modulator stuff
#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_MOD
#   define MO_DEBUG_MOD(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_MOD(unused__) { }
#endif

// for parameter updates between gui/scene
#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_PARAM
#   define MO_DEBUG_PARAM(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_PARAM(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_GUI
#   define MO_DEBUG_GUI(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_GUI(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_AUDIO
#   define MO_DEBUG_AUDIO(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_AUDIO(unused__) { }
#endif


#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_RENDER
#   define MO_DEBUG_RENDER(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_RENDER(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_IMG
#   define MO_DEBUG_IMG(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_IMG(unused__) { }
#endif

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_GEOM
#   define MO_DEBUG_GEOM(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_GEOM(unused__) { }
#endif

#if (1) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_SND
#   define MO_DEBUG_SND(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_SND(unused__) { }
#endif

#endif // MOSRC_IO_LOG_H
