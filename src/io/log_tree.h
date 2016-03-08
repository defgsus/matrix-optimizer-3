/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/8/2016</p>
*/

#ifndef MOSRC_IO_LOG_TREE_H
#define MOSRC_IO_LOG_TREE_H

#include "log.h"

#if (0) && defined(MO_ENABLE_DEBUG)
#   define MO_DO_DEBUG_TREE
#   define MO_DEBUG_TREE(stream_arg__) MO_DEBUG_IMPL_(stream_arg__)
#else
#   define MO_DEBUG_TREE(unused__) { }
#endif

#endif // MOSRC_IO_LOG_TREE_H

