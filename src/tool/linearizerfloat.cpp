/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <QList>

#include "linearizerfloat.h"


namespace MO {


struct LinearizerFloat::Private
{
    Private(LinearizerFloat*p)
        : p             (p)
        , lastInTime    (0.)
    {
    }

    struct TValue
    {
        TValue() { }
        TValue(Double t, Float v) : time(t), value(v) { }

        Double time;
        Float value;
    };

    void insertValue(Double time, Float value);
    Float getValue(Double time);

    LinearizerFloat * p;
    QReadWriteLock mutex;
    QList<TValue> list;
    Double lastInTime;
};


LinearizerFloat::LinearizerFloat()
    : p_    (new Private(this))
{

}

LinearizerFloat::~LinearizerFloat()
{
    delete p_;
}

void LinearizerFloat::insertValue(Double time, Float value)
{
    p_->insertValue(time, value);
}

Float LinearizerFloat::getValue(Double time) const
{
    return p_->getValue(time);
}


void LinearizerFloat::Private::insertValue(Double time, Float value)
{
    if (time > lastInTime)
    {
        QWriteLocker lock(&mutex);

        lastInTime = time;
        list << TValue(time, value);
    }
}

Float LinearizerFloat::Private::getValue(Double time)
{
    QReadLocker lock(&mutex);

    if (list.isEmpty())
        return 0.f;

    if (time <= list.first().time)
        return list.first().value;

    if (time >= list.last().time)
        return list.last().value;

    // find position in list
    auto i = list.end();
    --i;

    while (i->time > time)
    {
        if (i == list.begin())
            break;
        --i;
    }

    return i->value;
}


/*   0.5      1.0     2.0
 *    |        |       |
 *
 */

} // namespace MO
