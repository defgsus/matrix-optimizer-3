/** @file currentthread.cpp

    @brief Display of current thread id

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include <map>
#include <mutex>

#include <QThread>

#include "CurrentThread.h"

namespace MO {

namespace {

    static std::map<ThreadId, QString> threadNames_;
    static std::mutex mapLock_;
}

ThreadId currentThreadId()
{
    Q_STATIC_ASSERT(sizeof(void*) == sizeof(ThreadId));

    return reinterpret_cast<ThreadId>(QThread::currentThreadId());
}

void setCurrentThreadName(const QString &name)
{
    std::lock_guard<std::mutex> lock(mapLock_);

    // Note: Don't use the insert() method because
    // we wan't to overwrite previous names (if thread get's reused)
    threadNames_[currentThreadId()] = name;
}

QString currentThreadName()
{
    ThreadId id = currentThreadId();

    {
        std::lock_guard<std::mutex> lock(mapLock_);
        auto i = threadNames_.find(id);
        if (i != threadNames_.end())
            return i->second;
    }

    return QString::number(currentThreadId());
}

} // namespace MO
