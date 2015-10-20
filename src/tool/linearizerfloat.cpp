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
#include "math/interpol.h"

namespace MO {


struct LinearizerFloat::Private
{
    Private(LinearizerFloat*p)
        : p             (p)
        , lastInTime    (-1.)
        , interpolMode  (MATH::IT_NONE)
    {
    }

    struct TValue
    {
        TValue() { }
        TValue(Double t, Double v) : time(t), value(v) { }

        Double time;
        Double value;
    };

    void insertValue(Double time, Double value);
    Double getValue(Double time);

    LinearizerFloat * p;
    QReadWriteLock mutex;
    QList<TValue> list;
    Double lastInTime;
    MATH::InterpolationType interpolMode;
};


LinearizerFloat::LinearizerFloat()
    : p_    (new Private(this))
{

}

LinearizerFloat::~LinearizerFloat()
{
    delete p_;
}

MATH::InterpolationType LinearizerFloat::interpolationType() const
{
    return p_->interpolMode;
}

void LinearizerFloat::setInterpolationMode(MATH::InterpolationType m)
{
    p_->interpolMode = m;
}

void LinearizerFloat::insertValue(Double time, Double value)
{
    p_->insertValue(time, value);
}

Double LinearizerFloat::getValue(Double time) const
{
    return p_->getValue(time);
}

void LinearizerFloat::clear()
{
    QWriteLocker lock(&p_->mutex);
    p_->list.clear();
    p_->lastInTime = -1.;
}

void LinearizerFloat::Private::insertValue(Double time, Double value)
{
    if (time > lastInTime)
    {
        QWriteLocker lock(&mutex);

        lastInTime = time;
        list.append( TValue(time, value) );

        // remove far history
        while (!list.isEmpty() && list.front().time + 60. < time)
            list.pop_front();
    }
}

Double LinearizerFloat::Private::getValue(Double time)
{
    QReadLocker lock(&mutex);

    if (list.isEmpty())
        return 0.;

    if (time <= list.first().time)
        return list.first().value;

    if (time >= list.last().time)
        return list.last().value;

    // find position in list
    auto i = list.end(), j = i;
    --i;

    while (i->time > time)
    {
        if (i == list.begin())
            break;
        j = i;
        --i;
    }

    if (interpolMode == MATH::IT_NONE || j == list.end())
        return i->value;

    const Double
            // time delta
            td = j->time - i->time,
            // interpolation step [0,1]
            t = (time - i->time) / std::max(0.00001, td);

    if (interpolMode == MATH::IT_LINEAR)
        return MATH::interpol_linear(t, i->value, j->value);
    else if (interpolMode == MATH::IT_SMOOTH)
        return MATH::interpol_smooth(t, i->value, j->value);
    else
        return MATH::interpol_smooth2(t, i->value, j->value);
}



} // namespace MO
