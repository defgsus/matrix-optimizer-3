/** @file

    @brief 1-dimensional timeline

    <p>(c) 2011-2014, stefan.berke@modular-audio-graphics.com</p>

    @version 2011/11/16 created (grabed together from kepler project)
    @version 2014/04/24 grabed from libmag
*/

#include <cmath> // for fabs()

#include <QFile>

#include "io/datastream.h"
#include "io/error.h"
#include "math/timeline1d.h"
#include "math/interpol.h"

namespace MO {
namespace MATH {


Timeline1d::Timeline1d()
    : RefCounted("Timeline1d")
{
    // no current point
    cur_ = 0;

    // no limits :)
    lowerLimit_ = upperLimit_ = 0;
}

Timeline1d::~Timeline1d()
{
    // nothing to do
    // data (TpList) will wipe out itself
}


Timeline1d::Timeline1d(const Timeline1d &other)
    :   RefCounted("Tineline1d")
    ,   data_   (other.data_),
        cur_    (0),
        lowerLimit_(other.lowerLimit_),
        upperLimit_(other.upperLimit_),
        lmin_   (other.lmin_),
        lmax_   (other.lmax_)
{

}

const Timeline1d& Timeline1d::operator = (const Timeline1d& other)
{
    data_ = other.data_;
    cur_ = 0;
    lowerLimit_ = other.lowerLimit_;
    upperLimit_ = other.upperLimit_;
    lmin_ = other.lmin_;
    lmax_ = other.lmax_;

    return *this;
}

bool Timeline1d::operator == (const Timeline1d& other) const
{
    return data_ == other.data_;
}

Double Timeline1d::get(Double time) const
{
    return limit_(getNoLimit(time));
}

Double Timeline1d::getNoLimit(Double time) const
{
    if (data_.empty()) return 0.0;

    const TpHash htime = hash(time);

    TpList::const_iterator i1 = data_.upper_bound(htime);

    bool
        isFirst = (i1==data_.begin()),
        isOver = (i1==data_.end()),
        isLast = isLastElement_(i1);

    // get the one before time
    if ( !isFirst && htime <= i1->first )
    {
        i1--;

        isFirst = (i1==data_.begin());
        isOver = (i1==data_.end());
        isLast = isLastElement_(i1);
    }

    Double ret;
    bool returnLast = false;

    // before first
    if ( (isFirst) && htime < i1->first )
    {
        switch (i1->second.type)
        {
            default:
                ret = i1->second.val;
                return ret;
            case TimelinePoint::CONSTANT_USER:
            case TimelinePoint::SYMMETRIC_USER:
            {
                const Timeline1d::Point *t1 = &i1->second;
                ret = t1->val + t1->d1 * (time - t1->t);
                return ret;
            }
        }
    }
    else
    // after last
    if (isOver)
    {
        if (!isFirst) i1--;
        returnLast = true;
    }
    else
    // on last
    if (isLast)
        returnLast = true;

    if (returnLast)
    {
        switch (i1->second.type)
        {
            default:
                ret = i1->second.val;
                return ret;
            case TimelinePoint::CONSTANT_USER:
            case TimelinePoint::SYMMETRIC_USER:
            {
                const Timeline1d::Point *t1 = &i1->second;
                ret = t1->val + t1->d1 * (time - t1->t);
                return ret;
            }
        }
    }

    switch (i1->second.type)
    {
        default:
        case TimelinePoint::CONSTANT:
            ret = i1->second.val;
            return ret;
        break;

        case TimelinePoint::CONSTANT_USER:
        {
            const Timeline1d::Point *t1 = &i1->second;
            Double
                t = (time - t1->t),
                f = t;
            ret = (t1->val + t1->d1 * f);

            return ret;
        }
        break;

        case TimelinePoint::LINEAR:
        {
            auto i2 = i1; i2++;
            const Timeline1d::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double f = (time-t1->t)/(t2->t - t1->t);

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }
        break;

        case TimelinePoint::SMOOTH:
        {
            auto i2 = i1; i2++;
            const Timeline1d::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double f = (time-t1->t)/(t2->t - t1->t);
            f = 3.0*f*f*(1.0-f) + f*f*f;

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }
        break;

        case TimelinePoint::SYMMETRIC_USER:
        {
            auto i2 = i1; ++i2;
            const Timeline1d::Point
                *p1 = &i1->second,
                *p2 = &i2->second;
            Double
                t1 = (time - p1->t),         // second since 1st cue
                t2 = (p2->t - time),         // second to 2nd cue
                f = t1 / (p2->t - p1->t);    // linear transition 1st->2nd
                f = f*f*(3.-2.*f);
            ret = (p1->val + p1->d1 * t1) * (1. - f)
                + (p2->val - p2->d1 * t2) * f
                    ;
            return ret;
        }
        break;

        case TimelinePoint::SYMMETRIC:
        {
            auto i2 = i1; i2++;
            const Timeline1d::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double
                t0 = (time-t1->t),
                t = t0/(t2->t - t1->t),
                tq = t*t,
                f = -2.0*tq*t + 3.0*tq;
            ret =
                  (1.0 - f) * (t1->val + t1->d1 * t0)
                +        f  * (t2->val - t2->d1 * (t2->t - time));

            return ret;
        }
        break;

        /** a variation of hermite with arbitrary derivatives
            http://paulbourke.net/miscellaneous/interpolation/ */
        //case TimelinePoint::HERMITE_USER:
        case TimelinePoint::HERMITE_USER:
        case TimelinePoint::HERMITE:
        {
            auto i2 = i1; i2++;
            const Timeline1d::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double
                t = (time-t1->t)/(t2->t - t1->t),
                tq2 = t*t,
                tq3 = tq2*t,
                tr1 = 2.0*tq3 - 3.0*tq2 + 1.0,
                tr2 = (tq3 - 2.0*tq2 + t) ;
            ret =
                (tr1)     * t1->val +
                (tr2)     * t1->d1  +
                (tq3-tq2) * t2->d1  +
                (1.0-tr1) * t2->val;

            return ret;
        }
        break;


        case TimelinePoint::SPLINE4:
        {
            // get adjacent points
            TpList::const_iterator i0,i2,i3;

            // only one point?
            if (isFirst && isLast)
            {
                ret = i1->second.val;
                return ret;
            }

            Double y0,y1,y2,y3, t1,t2;
            y0=y1=y2=y3=i1->second.val;
            t1=i1->second.t; t2=t1+1.0;

            // only two points?
            if (data_.size()<3)
            {
                // then it's i1 and i2
                i2 = i1; i2++;
                y2 = i2->second.val;
                t2 = i2->second.t;
            }

            else
            {
                // three points minimum: i1,i2,i3

                i2 = i1; i2++;
                t2=i2->second.t;
                y2=i2->second.val;

                i3 = i2; i3++;
                // i3 missing?
                if (i3==data_.end())
                    y3 = y2;
                else
                    y3=i3->second.val;

                // i0 missing?
                if (isFirst)
                    y0 = y1;
                else
                {
                    i0 = i1; i0--;
                    y0 = i0->second.val;
                }
            }

            // interpolate between i1,i2 in i0,11,i2,i3

            Double
                t = (time - t1) / (t2-t1),
                tq = t*t,

                a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3,
                a1 = y0 - 2.5*y1 + 2.0*y2 - 0.5*y3,
                a2 = -0.5*y0 + 0.5*y2;


            ret = (a0*t*tq + a1*tq + a2*t + y1);
            return ret;
        }
        break;


        case TimelinePoint::SPLINE6:
        {
            // only one point?
            if (isFirst && isLast)
            {
                ret = i1->second.val;
                return ret;
            }

            // get adjacent points
            TpList::const_iterator im;
            // we want to interpolate between y2 and y3
            Double
                y2 = i1->second.val,y0=y2,y1=y2,y3=y2,y4=y2,y5=y2,
                t2=i1->second.t, t3=t2+1.0;

            im = i1;
            if (im != data_.begin())
            {
                --im;
                y1 = im->second.val;
                if (im != data_.begin())
                {
                    --im;
                    y0 = im->second.val;
                }
            }
            im = i1;
            ++im;
            if (im != data_.end())
            {
                y3 = im->second.val;
                t3 = im->second.t;
                ++im;
                if (im != data_.end())
                {
                    y4 = im->second.val;
                    if (im != data_.end())
                    {
                        ++im;
                        if (im != data_.end())
                            y5 = im->second.val;
                    }
                }
            }

            return interpol_6((time - t2) / (t3-t2), y0,y1,y2,y3,y4,y5);
        }
        break;

    }
}

Double Timeline1d::derivative(Double time, Double h) const
{
    return (get(time+h*.5) - get(time-h*.5)) / h;
}




void Timeline1d::clear()
{
    data_.clear();
}

void Timeline1d::addTimeline(const Timeline1d& tl, Double timeOffset)
{
    for (const auto & p : tl.data_)
    {
        add(p.second.t + timeOffset, p.second.val, p.second.type);
    }
}

void Timeline1d::overwriteTimeline(const Timeline1d& tl, Double timeOffset)
{
    remove(tl.tmin() + timeOffset, tl.tmax() + timeOffset);

    for (const auto & p : tl.data_)
    {
        add(p.second.t + timeOffset, p.second.val, p.second.type);
    }
}

Timeline1d::Point* Timeline1d::add(Double time, Double value, TimelinePoint::Type typ)
{
    // check if present
    TpList::iterator i = find(time);
    if (i!=data_.end())
    {
        return 0;
    }

    // create point
    Point p;

    p.t = time;
    p.val = value;
    p.type = typ;

    if (typ == TimelinePoint::DEFAULT)
        p.type = currentType_(time);
    else
        p.type = typ;

    p.d1 = p.isUserDerivative() ? derivative(time) : 0.0;

    // insert as element
    cur_ = &data_[hash(time)];
    *cur_ = p;

    // automatically set the derivative
    if (p.isAutoDerivative())
    {
        TpList::iterator i = first(p.t);
        setAutoDerivative(i);
    }

    return cur_;
}



Timeline1d::Point* Timeline1d::add(Double time, Double value, Double thresh, TimelinePoint::Type typ)
{
    Double v = get(time);
    if (value>=v-thresh && value<=v+thresh) return 0;

    return add(time, value, typ);
}

Timeline1d::Point* Timeline1d::add(Point &p)
{
    add(p.t, p.val, p.type);

    //cur_->d1 = p.d1;

    return cur_;
}

TimelinePoint::Type Timeline1d::currentType_(Double time)
{
    if (data_.empty())
        return TimelinePoint::SYMMETRIC_USER;

    TpList::iterator i = first(time);
    if (i==data_.end()) i--;
    if (i!=data_.begin() && i->second.t>=time) i--;

    return i->second.type;
}


void Timeline1d::remove(Double time)
{
    // check if present
    TpList::iterator i = find(time);
    if (i==data_.end()) return;

    // reset cur if it was pointing to that point
    if (cur_==&i->second) cur_ = 0;

    data_.erase(i);
}

void Timeline1d::remove(TpHash hash)
{
    // check if present
    TpList::iterator i = data_.lower_bound(hash);
    if (i==data_.end()) return;

    // reset cur if it was pointing to that point
    if (cur_==&i->second) cur_ = 0;

    data_.erase(i);
}

void Timeline1d::remove(Double start, Double end)
{
    if (data_.empty())
        return;

    auto s = first(start);
    if (s == data_.end())
        return;

    auto e = first(end);
    if (e == data_.end())
        --e;

    data_.erase(s, e);
}


void Timeline1d::setAutoDerivative(TpList::iterator &i)
{
    // prev, next
    TpList::iterator i0,i1;
    Double t0,t1,v0,v1;

    i0 = i;
    if (i0==data_.begin())
    {
        t0 = i->second.t-1.0;
        v0 = i->second.val;
    }
    else
    {
        i0--;
        t0 = i0->second.t;
        v0 = i0->second.val;
    }

    i1 = i;
    i1++;
    if (i1==data_.end())
    {
        t1 = i->second.t+1.0;
        v1 = i->second.val;
    }
    else
    {
        t1 = i1->second.t;
        v1 = i1->second.val;
    }

    i->second.d1 = (v1-v0)/(t1-t0);

    //qDebug() << "der" << v0 << v1 << i->second.t << i->second.d1;
}

void Timeline1d::setAutoDerivative(TpList::iterator start, TpList::iterator end)
{
    for (;start!=end;start++)
        setAutoDerivative(start);
}


void Timeline1d::shiftTime(Double secOff)
{
    // first, copy all points
    std::list<Timeline1d::Point> plist;

    for (TpList::iterator i=data_.begin();
            i != data_.end(); i++)
    {
        plist.push_back( i->second );
    }

    // wipe them out
    clear();

    // reinsert with offset
    for (std::list<Timeline1d::Point>::iterator i = plist.begin();
            i != plist.end(); i++)
    {
        add(i->t + secOff, i->val, i->type);
    }
}


void Timeline1d::serialize(IO::DataStream & stream)
{
    // version
    stream.writeHeader("timeline", 1);

    // write type enums
    stream << (quint8)TimelinePoint::MAX;
    for (quint8 i=0; i<TimelinePoint::MAX; ++i)
    {
        stream << QString(TimelinePoint::getPersistentName((TimelinePoint::Type)i));
    }

    // number of points
    stream << (quint64)data_.size();

    // write all points
    for (auto &i : data_)
    {
        stream << (quint8)i.second.type << i.second.t << i.second.val << i.second.d1;
    }
}

void Timeline1d::deserialize(IO::DataStream & stream)
{
    // check version
    //auto ver =
    stream.readHeader("timeline", 1);

    // get type enums
    quint8 numEnums;
    stream >> numEnums;

    std::map<quint8, TimelinePoint::Type> enumMap;
    for (quint8 i=0; i<numEnums; ++i)
    {
        QString name;
        stream >> name;
        enumMap.insert(std::make_pair(i, TimelinePoint::getTypeForPersistentName(name)));
    }

    // read points
    quint64 num;
    stream >> num;

    clear();

    for (quint64 i=0; i<num; ++i)
    {
        quint8 e;
        Point p;
        stream >> e >> p.t >> p.val >> p.d1;
        p.type = enumMap[e];

        // directly insert to avoid calculating derivatives
        cur_ = &data_[hash(p.t)];
        *cur_ = p;
    }
}


void Timeline1d::saveFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        MO_IO_ERROR(WRITE, "Timeline1D can't open file for writing '" << filename << "'\n"
                        << f.errorString());
    IO::DataStream stream(&f);

    serialize(stream);

    f.close();
}


void Timeline1d::loadFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Timeline1D can't open file for reading '" << filename << "'\n"
                            << f.errorString());
    IO::DataStream stream(&f);

    try
    {
        deserialize(stream);
    }
    catch (Exception & e)
    {
        e << "\nfrom file '" << filename << "'";
        throw;
    }

    f.close();
}








void Timeline1d::normalize(Double amp)
{
    Double ma = 0.0;
    for (TpList::iterator i=data_.begin();
            i!=data_.end(); i++)
    {
        ma = std::max(ma, fabs(i->second.val));
    }

    if (ma>0.0)
    for (TpList::iterator i=data_.begin();
            i!=data_.end(); i++)
    {
        i->second.val = i->second.val / ma * amp;
    }
}




Timeline1d::TpList::iterator Timeline1d::closest(Double t)
{
    if (data_.empty()) return data_.end();

    // check for last point
    TpList::iterator i = data_.end(); --i;
    if (i->second.t <= t) return i;

    // i >= t
    i = first(t);
    if (i==data_.end()) return i;

    if (i==data_.begin()) return i;

    // see if previous is closer
    TpList::iterator j=i; --j;

    return ( (t - j->second.t) <= (i->second.t - t) )? j : i;
}


void Timeline1d::getMinMax(Double tStart, Double tEnd, Double& minimal, Double& maximal)
{
    auto i = closest(tStart);

    // break early
    if (i == data_.end())
    {
        minimal = maximal = 0.0;
        return;
    }

    minimal = maximal = limit_(i->second.val);

    auto last = closest(tEnd);

    // XXX may need some finetuning
    if (last == data_.end())
        return;

    if (i->second.t >= last->second.t)
        return;

    // check each point
    while (i != last)
    {
        ++i;
        if (i == data_.end()) // safety
            return;

        const Double v = limit_(i->second.val);
        minimal = std::min(minimal, v);
        maximal = std::max(maximal, v);
    }

}

} // namespace MATH
} // namespace MO
