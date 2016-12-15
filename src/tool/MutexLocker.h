#ifndef MOSRC_TOOL_MUTEXLOCKER_H
#define MOSRC_TOOL_MUTEXLOCKER_H

#include <mutex>

//#define MO_ENABLE_MUTEX_TRACE

namespace MO {

#ifdef MO_ENABLE_MUTEX_TRACE
namespace Private
{
    void record_lock(void* ptr, const char* reason);
    void record_lock_succeeded(void* ptr, const char* reason);
    void record_unlock(void* ptr, const char* reason);
}
#endif


#ifdef MO_ENABLE_MUTEX_TRACE
void dumpMutexInfo(std::ostream& out);
#endif

/** Classic scoped lock around a mutex.
    Type M needs to support lock() and unlock()
*/
template <class M>
class MutexLockerT
{
    M& p_m_;
    bool p_locked_;
#ifdef MO_ENABLE_MUTEX_TRACE
    const char * p_reason_;
#endif
public:
    MutexLockerT(M& m, const char* reason)
        : p_m_      (m)
        , p_locked_ (false)
#ifdef MO_ENABLE_MUTEX_TRACE
        , p_reason_ (reason)
#endif
    {
#ifdef MO_ENABLE_MUTEX_TRACE
        Private::record_lock((void*)&p_m_, p_reason_);
        p_m_.lock();
        Private::record_lock_succeeded((void*)&p_m_, p_reason_);
#else
        (void)reason;
        p_m_.lock();
#endif
        p_locked_ = true;
    }

    ~MutexLockerT() throw()
    {
        if (p_locked_)
        {
#ifdef MO_ENABLE_MUTEX_TRACE
            Private::record_unlock((void*)&p_m_, p_reason_);
#endif
            p_m_.unlock();
        }
    }

    /** Unlocks the mutex before destructor */
    void unlock() throw()
    {
        if (p_locked_)
        {
#ifdef MO_ENABLE_MUTEX_TRACE
            Private::record_unlock((void*)&p_m_, p_reason_);
#endif
            p_m_.unlock();
            p_locked_ = false;
        }
    }
};


/** Classic scoped lock around a std::mutex */
typedef MutexLockerT<std::mutex> MutexLocker;
/** Classic scoped lock around a std::recursive_mutex */
typedef MutexLockerT<std::recursive_mutex> MutexLockerR;

} // namespace MO

#endif // MOSRC_TOOL_MUTEXLOCKER_H

