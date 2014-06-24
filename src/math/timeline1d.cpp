/** @file

    @brief 1-dimensional timeline

    <p>(c) 2011-2014, stefan.berke@modular-audio-graphics.com</p>

    @version 2011/11/16 created (grabed together from kepler project)
    @version 2014/04/24 grabed from libmag
*/
#include <cmath> // for fabs()
//#include <QDebug>

#include "math/timeline1d.h"
#include "math/interpol.h"

namespace MO {

const char *Timeline1D::Point::getName(Point::Type type)
{
    switch (type)
    {
        default:
        case Timeline1D::Point::DEFAULT:     return "default";    break;
        case Timeline1D::Point::CONSTANT:    return "constant";   break;
        case Timeline1D::Point::LINEAR:      return "linear";     break;
        case Timeline1D::Point::SMOOTH:      return "smooth";     break;
        case Timeline1D::Point::SYMMETRIC:   return "symmetric";  break;
        case Timeline1D::Point::SYMMETRIC2:  return "hermite";    break;
        case Timeline1D::Point::SPLINE4_SYM: return "symmetric4"; break;
        case Timeline1D::Point::SPLINE4:     return "spline4";    break;
        case Timeline1D::Point::SPLINE6:     return "spline6";    break;
    }
}

const char *Timeline1D::Point::getPersistentName(Point::Type type)
{
    switch (type)
    {
        default:
        case Timeline1D::Point::DEFAULT:     return "def";     break;
        case Timeline1D::Point::CONSTANT:    return "const";   break;
        case Timeline1D::Point::LINEAR:      return "linear";  break;
        case Timeline1D::Point::SMOOTH:      return "smooth";  break;
        case Timeline1D::Point::SYMMETRIC:   return "sym";     break;
        case Timeline1D::Point::SYMMETRIC2:  return "hermite"; break;
        case Timeline1D::Point::SPLINE4_SYM: return "sym4";    break;
        case Timeline1D::Point::SPLINE4:     return "spline4"; break;
        case Timeline1D::Point::SPLINE6:     return "spline6"; break;
    }
}


Timeline1D::Timeline1D()
{
    // no current point
    cur_ = 0;

    // no limits :)
    lowerLimit_ = upperLimit_ = 0;
}

Timeline1D::~Timeline1D()
{
    // nothing to do
    // data (TpList) will wipe out itself
}



Double Timeline1D::get(Double time)
{
    return limit_(getNoLimit(time));
}

Double Timeline1D::getNoLimit(Double time)
{
    if (data_.empty()) return 0.0;

    const TpHash htime = hash(time);

    TpList::iterator i1 = data_.upper_bound(htime);

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
        case Point::CONSTANT:
            ret = i1->second.val;
            return ret;
        break;

        case Point::LINEAR:
        {
            TpList::iterator i2 = i1; i2++;
            if (i2==data_.end()) {
                ret = i1->second.val;
                return ret;
            }
            Timeline1D::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double f = (time-t1->t)/(t2->t - t1->t);

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }
        break;

        case Point::SMOOTH:
        {
            TpList::iterator i2 = i1; i2++;
            if (i2==data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            Timeline1D::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double f = (time-t1->t)/(t2->t - t1->t);
            f = 3.0*f*f*(1.0-f) + f*f*f;

            ret = t1->val*(1.0-f) + f*(t2->val);
            return ret;
        }
        break;

        case Point::SYMMETRIC:
        {
            TpList::iterator i2 = i1; i2++;
            if (i2==data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            Timeline1D::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double
                t0 = (time-t1->t),
                t = t0/(t2->t - t1->t),
                tq = t*t,
                f = -2.0*tq*t + 3.0*tq;
            ret =
                  (1.0 - f) * (t1->val + t1->d1 * t0)
                +        f  * (t2->val - t2->d1 * (t2->t-time));

            return ret;
        }
        break;

        /** a variation of hermite with arbitrary derivatives
            http://paulbourke.net/miscellaneous/interpolation/ */
        case Point::SYMMETRIC2:
        {
            TpList::iterator i2 = i1; i2++;
            if (i2==data_.end())
            {
                ret = i1->second.val;
                return ret;
            }
            Timeline1D::Point
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


        case Point::SPLINE4_SYM:
        {
            TpList::iterator i0=i1, i2 = i1; i2++;
            if (i2==data_.end())
            {
                ret = i1->second.val;
                return ret;
            }

            Double d1 = 0, d2 = 0;

            if (i0!=data_.begin())
            {
                i0--;
                d1 = (i2->second.val-i0->second.val)/(i2->second.t-i0->second.t);
            }
            TpList::iterator i3=i2; i3++;
            if (i3!=data_.end())
            {
                d2 = (i3->second.val-i1->second.val)/(i3->second.t-i1->second.t);
            }

            Timeline1D::Point
                *t1 = &i1->second,
                *t2 = &i2->second;
            Double
                tt = (time-t1->t),
                t = tt/(t2->t - t1->t),
                tq = t*t,
                f = -2.0*tq*t + 3.0*tq;
            ret =
                  (1.0 - f) * (t1->val + d1 * tt)
                +        f  * (t2->val - d2 * (t2->t-time));

            return ret;
        }
        break;


        case Point::SPLINE4:
        {
            // get adjacent points
            TpList::iterator i0,i2,i3;

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


        case Point::SPLINE6:
        {
            // only one point?
            if (isFirst && isLast)
            {
                ret = i1->second.val;
                return ret;
            }

            // get adjacent points
            TpList::iterator im;
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





void Timeline1D::clear()
{
    data_.clear();
}


Timeline1D::Point* Timeline1D::add(Double time, Double value, Point::Type typ)
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
    p.d1 = 0.0;

    if (typ == Point::DEFAULT)
        p.type = currentType_(time);
    else
        p.type = typ;

    // insert as element
    cur_ = &data_[hash(time)];
    *cur_ = p;

    // automatically set the derivative
    if (hasAutoDerivative(p.type))
    {
        TpList::iterator i = first(p.t);
        setAutoDerivative(i);
    }

    return cur_;
}



Timeline1D::Point* Timeline1D::add(Double time, Double value, Double thresh, Point::Type typ)
{
    Double v = get(time);
    if (value>=v-thresh && value<=v+thresh) return 0;

    return add(time, value, typ);
}

Timeline1D::Point* Timeline1D::add(Point &p)
{
    add(p.t, p.val, p.type);

    //cur_->d1 = p.d1;

    return cur_;
}

Timeline1D::Point::Type Timeline1D::currentType_(Double time)
{
    if (data_.empty()) return Point::SYMMETRIC;

    TpList::iterator i = first(time);
    if (i==data_.end()) i--;
    if (i!=data_.begin() && i->second.t>=time) i--;

    return i->second.type;
}


void Timeline1D::remove(Double time)
{
    // check if present
    TpList::iterator i = find(time);
    if (i==data_.end()) return;

    // reset cur if it was pointing to that point
    if (cur_==&i->second) cur_ = 0;

    data_.erase(i);
}

void Timeline1D::remove(TpHash hash)
{
    // check if present
    TpList::iterator i = data_.lower_bound(hash);
    if (i==data_.end()) return;

    // reset cur if it was pointing to that point
    if (cur_==&i->second) cur_ = 0;

    data_.erase(i);
}


void Timeline1D::setAutoDerivative(TpList::iterator &i)
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

void Timeline1D::setAutoDerivative(TpList::iterator start, TpList::iterator end)
{
    for (;start!=end;start++)
        setAutoDerivative(start);
}


void Timeline1D::shiftTime(Double secOff)
{
    // first, copy all points
    std::list<Timeline1D::Point> plist;

    for (TpList::iterator i=data_.begin();
            i != data_.end(); i++)
    {
        plist.push_back( i->second );
    }

    // wipe them out
    clear();

    // reinsert with offset
    for (std::list<Timeline1D::Point>::iterator i = plist.begin();
            i != plist.end(); i++)
    {
        add(i->t + secOff, i->val, i->type);
    }
}






bool Timeline1D::saveFile(const char *filename)
{
    FILE *f = fopen( filename, "w" );
    if (!f) return false;

    saveFile(f);

    fclose(f);
    return true;
}

void Timeline1D::saveFile(FILE *f)
{
    fprintf(f, "timeline1d { ");
    // version
    fprintf(f, "1 ");
    // nr of points
    fprintf(f, "%ld ", data_.size() );
    // list
    for (TpList::iterator i=data_.begin(); i!=data_.end(); i++)
    {
        fprintf(f, "%d %g %g ", i->second.type, i->second.t, i->second.val);
    }
    fprintf(f, "} ");
}

bool Timeline1D::loadFile(const char *filename)
{
    FILE *f = fopen( filename, "r" );

    if (!f)	return false;

    bool result = loadFile(f);

    fclose(f);

    return result;
}

bool Timeline1D::loadFile(FILE *f)
{
    int e = fscanf(f, "timeline1d { ");

    // version
    int ver;
    e = fscanf(f, "%d", &ver);
    if (e!=1) return false;

    // number of points
    int nr;
    e = fscanf(f, "%d ", &nr );
    if (e!=1) return false;

    clear();

    int typ;
    Double t,val;
    for (int i=0; i<nr; i++)
    {
        e = fscanf(f, "%d %lg %lg ", &typ, &t, &val);
        if (e!=3) return false;
        add(t, val, (Timeline1D::Point::Type)typ);
    }
    e = fscanf(f, "} ");

    return true;
}







void Timeline1D::normalize(Double amp)
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




Timeline1D::TpList::iterator Timeline1D::closest(Double t)
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


void Timeline1D::getMinMax(Double tStart, Double tEnd, Double& minimal, Double& maximal)
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

} // namespace MO
