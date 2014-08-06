/** @file currentthread.h

    @brief Display of current thread id

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_IO_CURRENTTHREAD_H
#define MOSRC_IO_CURRENTTHREAD_H

#include <QString>

namespace MO {

typedef unsigned long int ThreadId;

ThreadId currentThreadId();

QString currentThreadName();

/** Sets the name of the current thread */
void setCurrentThreadName(const QString& name);

} // namespace MO

#endif // MOSRC_IO_CURRENTTHREAD_H
