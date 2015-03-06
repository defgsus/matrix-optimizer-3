/** @file lockedoutput.h

    @brief Threadsafe streamout

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#ifndef MOSRC_IO_LOCKEDOUTPUT_H
#define MOSRC_IO_LOCKEDOUTPUT_H

#define MO_ENABLE_LOCKED_OUTPUT

#include <iostream>

#ifdef MO_ENABLE_LOCKED_OUTPUT
    #include <mutex>

    namespace MO {
    namespace Private {

        /** singelton scoped lock for string output */
        class IoLock
        {
        public:
            IoLock();
            ~IoLock();
        };

    } // namespace Private
    } // namespace MO
#endif


#ifdef MO_ENABLE_LOCKED_OUTPUT
#   define MO_STREAM_OUT(out__, stream_arg__) \
        { ::MO::Private::IoLock lock; out__ << stream_arg__; }
#else
        MO_STREAM_OUT(out__, stream_arg__) \
                { out__ << stream_arg__; }
#endif



#endif // MOSRC_IO_LOCKEDOUTPUT_H
