/** @file threadpool.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.04.2015</p>
*/

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include "threadpool.h"
#include "io/log.h"

namespace MO {


class ThreadPool::Thread : public QThread
{
public:

    Thread(ThreadPool * p) : QThread(), pool(p), func(0) { }

    bool isBusy() const { return func ? true : false; }

    void run() Q_DECL_OVERRIDE
    {
        doStop = false;

        func = 0;

        while (!doStop)
        {
            if (!func)
                msleep(10);
            else
                func();

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

void ThreadPool::start(int numberThreads)
{
    if (isRunning())
        return;

    if (numberThreads <= 0)
        numberThreads = QThread::idealThreadCount();

    QMutexLocker lock(mutex_);

    for (int i=0; i<numberThreads; ++i)
        threads_.push_back( new Thread(this) );

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


} // namespace MO
