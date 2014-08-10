/** @file lockedoutput.cpp

    @brief Threadsafe streamout

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#include "lockedoutput.h"

#ifdef MO_ENABLE_LOCKED_OUTPUT

    namespace MO {
    namespace Private {

        std::mutex IoLock::mutex_;

        IoLock::IoLock()
        {
            mutex_.lock();
        }

        IoLock::~IoLock()
        {
            mutex_.unlock();
        }

    } // namespace Private
    } // namespace MO

#endif
