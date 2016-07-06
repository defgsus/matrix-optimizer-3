/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#include <QFile>

#include "math/timelinend.h"
#include "math/timeline1d.h"
#include "math/interpol.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace MATH {



TimelineNd::TimelineNd(size_t dim)
    : RefCounted("TimelineNd")
    , p_dim_        (dim)
    , p_cur_          (0)
    , p_isLowerLimit_   (false)
    , p_isUpperLimit_   (false)
    , p_minVal_         (dim)
    , p_maxVal_         (dim)
{
    MO_PRINT("TimelineNd("<<this<<")");
}


TimelineNd::TimelineNd(const TimelineNd &other)
    :   RefCounted("TimelineNd")
        , p_data_       (other.p_data_),
        p_dim_      (other.p_dim_),
        p_cur_        (0),
        p_isLowerLimit_ (other.p_isLowerLimit_),
        p_isUpperLimit_ (other.p_isUpperLimit_),
        p_minVal_       (other.p_minVal_),
        p_maxVal_       (other.p_maxVal_)
{
    //MO_PRINT("TimelineNd("<<this<<")");
}

TimelineNd::~TimelineNd()
{
    //MO_PRINT("~TimelineNd("<<this<<")");
}

const TimelineNd& TimelineNd::operator = (const TimelineNd& other)
{
    p_data_ = other.p_data_;
    p_cur_ = 0;
    p_isLowerLimit_ = other.p_isLowerLimit_;
    p_isUpperLimit_ = other.p_isUpperLimit_;
    p_minVal_ = other.p_minVal_;
    p_maxVal_ = other.p_maxVal_;

    return *this;
}
/*
bool TimelineNd::operator == (const TimelineNd& other) const
{
    return data_ == other.data_;
}
*/
TimelineNd::ValueType TimelineNd::get(Double time) const
{
    auto r = getNoLimit(time);
    p_limit_(r);
    return r;
}

TimelineNd::ValueType TimelineNd::getDerivative(Double time, Double h) const
{
#if 0 // XXX stimmt nich!

    Double  h2 = .5 * h;
    auto    d0 = get(time) * 2.,
            d1 = get(time - h2),
            d2 = get(time + h2);
    d2 -= d0;
    d2 += d1;
    d2 /= (h * h);
    return d2;
    //return (d2 - 2. * d0 + d1) / (h * h);
#else
    return (get(time+h*.5) - get(time-h*.5)) / h;
#endif
}

TimelineNd::ValueType TimelineNd::getNoLimit(Double time) const
{
    if (p_data_.empty())
        return ValueType(p_dim_, 0);

    const TpHash htime = hash(time);

    TpList::const_iterator i1 = p_data_.upper_bound(htime);

    bool
        isFirst = (i1==p_data_.begin()),
        isOver = (i1==p_data_.end()),
        isLast = p_isLastElement_(i1);

    // get the one before time
    if ( !isFirst && htime <= i1->first )
    {
        i1--;

        isFirst = (i1==p_data_.begin());
        isOver = (i1==p_data_.end());
        isLast = p_isLastElement_(i1);
    }

    ValueType ret(0);

    // before first
    if ( (isFirst) && htime < i1->first )
    {
        ret = i1->second.val;
        return ret;
    }
    else
    // after last
    if (isOver)
    {
        if (!isFirst) i1--;
        ret = i1->second.val;
        return ret;
    }
    else
    // on last
    if (isLast)
    {
        ret = i1->second.val;
        return ret;
    }

    switch (i1->second.type)
    {
        default:
        case TimelinePoint::CONSTANT:
            ret = i1->second.val;
            return ret;

        case TimelinePoint::LINEAR:
        {
            auto i2 = i1; i2++;
            if (i2==p_data_.end()) {
                ret = i1->second.val;
                return ret;
            }
            const TimelineNd::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double f = (time-t1->t)/(t2->t - t1->t);

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }

        case TimelinePoint::SMOOTH:
        {
            auto i2 = i1; i2++;
            if (i2==p_data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            const TimelineNd::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            ValueType f = (time-t1->t)/(t2->t - t1->t);
            f = 3.0*f*f*(1.0-f) + f*f*f;

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }

        case TimelinePoint::SYMMETRIC:
        case TimelinePoint::SYMMETRIC_USER:
        {
            auto i2 = i1; i2++;
            if (i2==p_data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            const TimelineNd::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double
                t0 = (time - t1->t),
                t = t0/(t2->t - t1->t),
                tq = t*t,
                f = -2.0*tq*t + 3.0*tq;
            ret =
                  (1.0 - f) * (t1->val + t1->d1 * t0)
                +        f  * (t2->val - t2->d1 * (t2->t-time));

            return ret;
        }

        /** a variation of hermite with arbitrary derivatives
            http://paulbourke.net/miscellaneous/interpolation/ */
        case TimelinePoint::HERMITE:
        {
            auto i2 = i1; i2++;
            if (i2==p_data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            const TimelineNd::Point
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

            ValueType
                    y0 = i1->second.val,
                    y1=y0, y2=y0, y3=y0;
            Double
                    t1=i1->second.t,
                    t2=t1+1.0;

            // only two points?
            if (p_data_.size()<3)
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
                if (i3==p_data_.end())
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
                tq = t*t;
            ValueType
                a0 = -0.5*y0 + 1.5*y1 - 1.5*y2 + 0.5*y3,
                a1 = y0 - 2.5*y1 + 2.0*y2 - 0.5*y3,
                a2 = -0.5*y0 + 0.5*y2;


            ret = (a0*t*tq + a1*tq + a2*t + y1);
            return ret;
        }


        case TimelinePoint::SPLINE6:
        {
            // only one point?
            if (isFirst && isLast)
            {
                return i1->second.val;
            }

            // get adjacent points
            TpList::const_iterator im;
            // we want to interpolate between y2 and y3
            ValueType
                y2 = i1->second.val,y0=y2,y1=y2,y3=y2,y4=y2,y5=y2;
            Double
                t2=i1->second.t, t3=t2+1.0;

            im = i1;
            if (im != p_data_.begin())
            {
                --im;
                y1 = im->second.val;
                if (im != p_data_.begin())
                {
                    --im;
                    y0 = im->second.val;
                }
            }
            im = i1;
            ++im;
            if (im != p_data_.end())
            {
                y3 = im->second.val;
                t3 = im->second.t;
                ++im;
                if (im != p_data_.end())
                {
                    y4 = im->second.val;
                    if (im != p_data_.end())
                    {
                        ++im;
                        if (im != p_data_.end())
                            y5 = im->second.val;
                    }
                }
            }

            return interpol_6((time - t2) / (t3-t2), y0,y1,y2,y3,y4,y5);
        }

    }
}





void TimelineNd::clear()
{
    p_data_.clear();
}

void TimelineNd::setDimensions(size_t dim)
{
    p_dim_ = dim;
    for (auto& i : p_data_)
    {
        i.second.val.setDimensions(dim);
        i.second.d1.setDimensions(dim);
    }
}

#if 0
void TimelineNd::addTimeline(const TimelineNd& tl, Double timeOffset)
{
    for (const auto & p : tl.data_)
    {
        add(p.second.t + timeOffset, p.second.val, p.second.type);
    }
}

void TimelineNd::overwriteTimeline(const TimelineNd& tl, Double timeOffset)
{
    remove(tl.tmin() + timeOffset, tl.tmax() + timeOffset);

    for (const auto & p : tl.data_)
    {
        add(p.second.t + timeOffset, p.second.val, p.second.type);
    }
}
#endif

TimelineNd::Point* TimelineNd::add(Double time, const ValueType& value, TimelinePoint::Type typ)
{
    // check if present
    TpList::iterator i = find(time);
    if (i!=p_data_.end())
    {
        return 0;
    }

    // create point
    Point p(p_dim_);

    p.t = time;
    p.val = value;
    p.type = typ;
    p.d1.fill(0.0);

    if (typ == TimelinePoint::DEFAULT)
        p.type = p_currentType_(time);
    else
        p.type = typ;

    // insert as element
    p_cur_ = &p_data_[hash(time)];
    *p_cur_ = p;

    // automatically set the derivative
    if (hasAutoDerivative(p.type))
    {
        TpList::iterator i = first(p.t);
        setAutoDerivative(i);
    }

    return p_cur_;
}


#if 0
TimelineNd::Point* TimelineNd::add(Double time, Double value, Double thresh, TimelinePoint::Type typ)
{
    Double v = get(time);
    if (value>=v-thresh && value<=v+thresh) return 0;

    return add(time, value, typ);
}
#endif

TimelineNd::Point* TimelineNd::add(Point &p)
{
    add(p.t, p.val, p.type);

    return p_cur_;
}

TimelinePoint::Type TimelineNd::p_currentType_(Double time)
{
    if (p_data_.empty())
        return TimelinePoint::SYMMETRIC;

    TpList::iterator i = first(time);
    if (i==p_data_.end()) i--;
    if (i!=p_data_.begin() && i->second.t>=time) i--;

    return i->second.type;
}


void TimelineNd::remove(Double time)
{
    // check if present
    TpList::iterator i = find(time);
    if (i==p_data_.end()) return;

    // reset cur if it was pointing to that point
    if (p_cur_==&i->second) p_cur_ = 0;

    p_data_.erase(i);
}

void TimelineNd::remove(TpHash hash)
{
    // check if present
    TpList::iterator i = p_data_.lower_bound(hash);
    if (i==p_data_.end()) return;

    // reset cur if it was pointing to that point
    if (p_cur_==&i->second) p_cur_ = 0;

    p_data_.erase(i);
}

void TimelineNd::remove(Double start, Double end)
{
    if (p_data_.empty())
        return;

    auto s = first(start);
    if (s == p_data_.end())
        return;

    auto e = first(end);
    if (e == p_data_.end())
        --e;

    p_data_.erase(s, e);
}


void TimelineNd::setAutoDerivative(TpList::iterator &i)
{
    // prev, next
    TpList::iterator i0,i1;
    Double t0,t1;
    ValueType v0(0), v1(0);

    i0 = i;
    if (i0==p_data_.begin())
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
    if (i1==p_data_.end())
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

void TimelineNd::setAutoDerivative(TpList::iterator start, TpList::iterator end)
{
    for (;start!=end;start++)
        setAutoDerivative(start);
}


void TimelineNd::shiftTime(Double secOff)
{
    // first, copy all points
    std::list<TimelineNd::Point> plist;

    for (TpList::iterator i=p_data_.begin();
            i != p_data_.end(); i++)
    {
        plist.push_back( i->second );
    }

    clear();

    // and reinsert with offset
    for (std::list<TimelineNd::Point>::iterator i = plist.begin();
            i != plist.end(); i++)
    {
        add(i->t + secOff, i->val, i->type);
    }
}

#if 0
void TimelineNd::serialize(IO::DataStream & stream)
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

void TimelineNd::deserialize(IO::DataStream & stream)
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


void TimelineNd::saveFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
        MO_IO_ERROR(WRITE, "TimelineNd can't open file for writing '" << filename << "'\n"
                        << f.errorString());
    IO::DataStream stream(&f);

    serialize(stream);

    f.close();
}


void TimelineNd::loadFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "TimelineNd can't open file for reading '" << filename << "'\n"
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
#endif



#if 0
void TimelineNd::normalize(Double amp)
{
    ValueType ma = 0.0;
    for (TpList::iterator i=data_.begin();
            i!=data_.end(); i++)
    {
        ma = std::max(ma, std::abs(i->second.val));
    }

    if (ma>0.0)
    for (TpList::iterator i=data_.begin();
            i!=data_.end(); i++)
    {
        i->second.val = i->second.val / ma * amp;
    }
}
#endif


Timeline1d* TimelineNd::getTimeline1d(size_t dimension)
{
    auto tl = new Timeline1d();

    for (auto i = p_data_.begin(); i != p_data_.end(); ++i)
        tl->add(i->second.t, i->second.val[dimension], i->second.type);

    return tl;
}

TimelineNd* TimelineNd::getTimelineNd(size_t first, size_t num)
{
    auto tl = new TimelineNd(num);
    if (!numDimensions())
        return tl;

    first = std::min(numDimensions()-1, first);
    num = std::min(numDimensions()-first, num);

    if (num)
    for (auto i = p_data_.begin(); i != p_data_.end(); ++i)
    {
        ValueType v(num);
        for (size_t k = 0; k < num; ++k)
            v[k] = i->second.val[k+first];

        tl->add(i->second.t, v, i->second.type);
    }

    return tl;
}

} // namespace MATH
} // namespace MO
