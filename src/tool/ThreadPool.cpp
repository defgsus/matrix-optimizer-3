/** @file threadpool.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.04.2015</p>
*/

#include <chrono>
#include <thread>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "ThreadPool.h"
#include "io/time.h"
#include "io/CurrentThread.h"
#include "io/log.h"

namespace MO {


class ThreadPool::Thread : public QThread
{
public:

    Thread(ThreadPool * p, long index)
        : QThread(), pool(p), func(0), index(index),
          accumTime(-1.), avTime(-1.), runs(0)
    { }

    bool isBusy() const { return func ? true : false; }

    /** Average time over all work */
    Double averageTimeAll() const { return runs < 1 || accumTime < 0 ? 0. : accumTime / runs; }
    /** Average time over last couple of work */
    Double averageTime() const { return avTime < 0 ? 0. : avTime; }

    void run() Q_DECL_OVERRIDE
    {
        setCurrentThreadName(QString("WORK%1").arg(index));

        doStop = false;        
        func = 0;
        accumTime = -1.;
        avTime = -1;
        runs = 0;

        TimeMessure tm;

        while (!doStop)
        {
            if (!func)
                msleep(10);
            else
            {
                tm.start();
                // execute
                func();
                // stats
                ++runs;
                Double t = tm.time();
                accumTime += t;
                if (avTime < 0.)
                    avTime = t;
                else
                    avTime += .2 * (t - avTime);
            }

            // query more work
            {
                QMutexLocker lock(pool->mutex_);

                if (pool->que_.isEmpty())
                {
                    func = 0;
                    continue;
                }
                else
                {
                    func = pool->que_[0];
                    pool->que_.pop_front();
                }
            }

        }
    }

    ThreadPool * pool;
    volatile bool doStop;
    std::function<void()> func;
    long index;
    Double accumTime, avTime;
    long runs;
};



ThreadPool::ThreadPool(QObject *parent)
    : QObject           (parent)
    , mutex_            (new QMutex(QMutex::Recursive))
    , running_          (false)
{
}

int ThreadPool::numberActiveThreads() const
{
    int n = 0;
    for (auto t : threads_)
        if (t->isBusy())
            ++n;
    return n;
}

Double ThreadPool::averageWorkTime() const
{
    if (threads_.empty())
        return 0.0;

    Double n = 0.;
    for (auto t : threads_)
        n += t->averageTime();
    return n / threads_.size();
}

void ThreadPool::start(int numberThreads)
{
    if (isRunning())
        return;

    if (numberThreads <= 0)
        numberThreads = QThread::idealThreadCount();

    QMutexLocker lock(mutex_);

    for (int i=0; i<numberThreads; ++i)
        threads_.push_back( new Thread(this, i) );

    for (auto t : threads_)
        t->start();
}

void ThreadPool::stop()
{
    for (auto t : threads_)
    {
        //MO_PRINT("stopping thread " << t);
        t->doStop = true;
    }

    {
        QMutexLocker lock(mutex_);

        que_.clear();
    }

    for (auto t : threads_)
    {
        t->wait();
        //MO_PRINT("thread " << t << " stopped");
        delete t;
    }
    threads_.clear();

    running_ = false;
}

void ThreadPool::addWork(std::function<void ()> func)
{
    QMutexLocker lock(mutex_);

    que_.push_back(func);
}

void ThreadPool::block(size_t maxQueSize)
{
    while (true)
    {
        {
            QMutexLocker lock(mutex_);
            if (size_t(que_.size()) <= maxQueSize)
                return;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace MO
