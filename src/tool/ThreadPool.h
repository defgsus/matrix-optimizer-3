/** @file threadpool.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.04.2015</p>
*/

#ifndef MOSRC_TOOL_THREADPOOL_H
#define MOSRC_TOOL_THREADPOOL_H

#include <functional>
#include <QObject>
#include <QList>

#include "types/float.h"

class QMutex;
class QThread;

namespace MO {


/** A pool of threads to work on separate data */
class ThreadPool : public QObject
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = 0);

    bool isRunning() const { return running_; }

    /** Number of parallel threads */
    int numberThreads() const { return threads_.size(); }
    /** Number of currently active threads */
    int numberActiveThreads() const;
    /** Number of work in the que */
    int numberWork() const { return que_.size(); }

    /** Average time of executed work in seconds */
    Double averageWorkTime() const;

signals:

public slots:

    /** Starts the threads.
        If @p numberThreads == 0, it will be set to the number of processors */
    void start(int numberThreads = 0);

    void stop();

    /** Put the function into the que */
    void addWork(std::function<void()> func);

    /** Blocks execution of the calling thread, until
        the number of work in the que is less than or equal to @p maxQueSize */
    void block(size_t maxQueSize);

private:

    class Thread;
    friend class Thread;
    QMutex * mutex_;
    QList<std::function<void()>> que_;
    QList<Thread*> threads_;
    bool running_;
};

} // namespace MO

#endif // MOSRC_TOOL_THREADPOOL_H
