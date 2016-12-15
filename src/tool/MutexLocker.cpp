#include <iostream>
#include <iomanip>
#include <map>

#include "MutexLocker.h"


#ifdef MO_ENABLE_MUTEX_TRACE
#include "time.h"
namespace MO {
namespace Private
{
    namespace
    {
        struct TimeInfo
        {
            double timeWaited, timeLocked;
            const char* reason;
            int64_t curLockCount,
                    lockCount,
                    lockWaitCount;
            TimeMessure tm;
        };

        struct MutexInfo
        {
            const void* ptr;
            int64_t curLockCount;
            std::map<const void*, TimeInfo*> timeInfoMap;
        };

        static std::map<const void*, MutexInfo*> infoMap;
        static std::mutex mapMutex;

        MutexInfo* getMutexInfo(const void* ptr)
        {
            auto i = infoMap.find(ptr);
            if (i != infoMap.end())
                return i->second;

            auto info = new MutexInfo();
            info->ptr = ptr;
            info->curLockCount = 0;
            infoMap.insert(std::make_pair(ptr, info));
            return info;
        }

        TimeInfo* getTimeInfo(MutexInfo* info, const char* reason)
        {
            auto i = info->timeInfoMap.find(reason);
            if (i != info->timeInfoMap.end())
                return i->second;
            auto tinfo = new TimeInfo;
            tinfo->timeLocked =
            tinfo->timeWaited = 0.;
            tinfo->lockCount =
            tinfo->curLockCount =
            tinfo->lockWaitCount = 0;
            tinfo->reason = reason;
            info->timeInfoMap.insert(std::make_pair(reason, tinfo));
            return tinfo;
        }
    }

    void record_lock(void* ptr, const char* reason)
    {
        mapMutex.lock();

        auto info = getMutexInfo(ptr);
        auto tinfo = getTimeInfo(info, reason);
        if (info->curLockCount > 0)
            ++tinfo->lockWaitCount;
        ++info->curLockCount;
        ++tinfo->lockCount;
        ++tinfo->curLockCount;
        tinfo->tm.start();

        mapMutex.unlock();
    }

    void record_lock_succeeded(void* ptr, const char* reason)
    {
        mapMutex.lock();

        auto info = getMutexInfo(ptr);
        auto tinfo = getTimeInfo(info, reason);
        tinfo->timeWaited += tinfo->tm.time();
        tinfo->tm.start();

        mapMutex.unlock();
    }

    void record_unlock(void* ptr, const char* reason)
    {
        mapMutex.lock();

        auto info = getMutexInfo(ptr);
        auto tinfo = getTimeInfo(info, reason);
        tinfo->timeLocked += tinfo->tm.time();
        --info->curLockCount;
        --tinfo->curLockCount;

        mapMutex.unlock();
    }
}

void dumpMutexInfo(std::ostream& out)
{
    // --- copy and sort mutex info ----

    struct Info
    {
        const void* mutex;
        const char* reason;
        double timeWaited, timeLocked;
        int64_t lockCount, lockWaitCount;
    };
    std::map<double, Info> infoMap;

    Private::mapMutex.lock();

    for (auto i = Private::infoMap.begin();
         i != Private::infoMap.end(); ++i)
    {
        Private::MutexInfo* info = i->second;

        for (auto j = info->timeInfoMap.begin();
             j != info->timeInfoMap.end(); ++j)
        {
            Private::TimeInfo* tinfo = j->second;
            Info inf;
            inf.mutex = i->first;
            inf.reason = tinfo->reason;
            inf.timeWaited = tinfo->timeWaited;
            inf.timeLocked = tinfo->timeLocked;
            inf.lockCount = tinfo->lockCount;
            inf.lockWaitCount = tinfo->lockWaitCount;
            infoMap.insert(std::make_pair(inf.timeWaited, inf));
        }
    }

    Private::mapMutex.unlock();


    out << std::setw(17) << "MUTEX"
        << std::setw(12) << "NUM LOCKS"
        << std::setw(12) << "NUM WAITED"
        << std::setw(14) << "TIME LOCKED"
        << std::setw(14) << "TIME WAITED"
        << " REASON"
        << std::endl;

    for (auto i = infoMap.begin(); i != infoMap.end(); ++i)
    {
        const Info& info = i->second;
        out << std::setw(17) << info.mutex
            << std::setw(12) << info.lockCount
            << std::setw(12) << info.lockWaitCount
            << std::setw(14) << info.timeLocked
            << std::setw(14) << info.timeWaited
            << " " << info.reason
            << std::endl;
    }

} // namespace Private
} // namespace MO

#endif

