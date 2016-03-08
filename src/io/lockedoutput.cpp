/** @file lockedoutput.cpp

    @brief Threadsafe streamout

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#include <cassert>
#include "lockedoutput.h"

#ifdef MO_ENABLE_LOCKED_OUTPUT

    namespace MO {
    namespace Private {

        static std::recursive_mutex* log_mutex_ = 0;

        IoLock::IoLock()
        {
            if (!log_mutex_)
                log_mutex_ = new std::recursive_mutex;
            log_mutex_->lock();
        }

        IoLock::~IoLock()
        {
            assert(log_mutex_ && "unlock without lock in guard class");
            log_mutex_->unlock();
        }

    } // namespace Private
    } // namespace MO

#endif
