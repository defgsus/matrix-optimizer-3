/** @file currentthread.cpp

    @brief Display of current thread id

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include <map>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "currentthread.h"

namespace MO {

namespace {

    static std::map<ThreadId, QString> threadNames_;
    static QMutex mapLock_;
}

ThreadId currentThreadId()
{
    Q_STATIC_ASSERT(sizeof(void*) == sizeof(ThreadId));

    return reinterpret_cast<ThreadId>(QThread::currentThreadId());
}

void setCurrentThreadName(const QString &name)
{
    QMutexLocker lock(&mapLock_);

    threadNames_.insert(std::make_pair(currentThreadId(), name));
}

QString currentThreadName()
{
    ThreadId id = currentThreadId();

    {
        QMutexLocker lock(&mapLock_);
        auto i = threadNames_.find(id);
        if (i != threadNames_.end())
            return i->second;
    }

    return QString::number(currentThreadId());
}

} // namespace MO
