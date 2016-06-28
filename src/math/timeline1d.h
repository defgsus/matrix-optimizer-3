/** @file

    @brief 1-dimensional timeline

    <p>(c) 2011-2014, stefan.berke@modular-audio-graphics.com</p>

    @version 2011/11/16 created (grabed together from kepler project)
    @version 2014/04/24 grabed from libmag
*/
#ifndef MOSRC_MATH_TIMELINE1D_H
#define MOSRC_MATH_TIMELINE1D_H

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include <QString>

#include "types/float.h"
#include "types/refcounted.h"
#include "timeline_point.h"

namespace MO {
namespace IO { class DataStream; }
namespace MATH {

/**	a super-duper timeline component mapping seconds to values.

    <p>currently the hash value for the time component of a point is <b>(long int)(time * 4096)</b>.
    that means you can set 4096 different points per one-second-of-timeline and time can be maximally
    +/- 524287 seconds or ~8738 minutes or ~145 hours.</p>
*/
class Timeline1d : public RefCounted
{
    public:

    // ------------ types -------------

    /** one que-point of a Timeline1D */
    struct Point
    {
        Double
            /** time of the que-point in seconds */
            t,
            /** value of the que-point */
            val,
            /** first derrivative of the point, user-adjustable */
            d1;

        /** type of this que-point, see MO::MATH::TimelinePoint::Type */
        TimelinePoint::Type type;

        /** Has the type user adjustable derivatives */
        bool isUserDerivative() const { return TimelinePoint::isUserDerivative(type); }
        bool isAutoDerivative() const { return TimelinePoint::isAutoDerivative(type); }
        bool hasDerivative() const { return TimelinePoint::hasDerivative(type); }
        bool isContinuous() const { return TimelinePoint::isContinuous(type); }

        bool operator == (const Point& o) const { return t == o.t && val == o.val && d1 == o.d1 && type == o.type; }
    };

    /** the type of hash-value generated for the time of a point. <br>
        <b>NOTE</b>: actually this is no random hash value. the points
        in TpList (more specific in Timeline1D::data_) are always ordered
        according to their time. TpHash is simply the integer version of
        the time multiplied by a reasonable large constant. */
    typedef int64_t TpHash;

    static const TpHash InvalidHash = 0xffffffffffffffff;

    /** Returns the hash value for a specific time */
    static TpHash hash(Double time) { return (TpHash)(time * 4096.0); }

    /** Returns the smallest possible change in time. @see TpHash */
    static Double timeQuantum() { return 1.0 / 4096.0; }

    /** the default container mapping between hashvalues and points */
    typedef std::map<TpHash, Point> TpList;

    // ------- ctor / dtor -----------

    /** default constructor, no parameters */
    Timeline1d();
    ~Timeline1d();

    // ------ copy assignment --------

    Timeline1d(const Timeline1d& other);
    const Timeline1d& operator = (const Timeline1d& other);

    // ----------- io ----------------

    /** Writes timeline to binary format. */
    void serialize(IO::DataStream&);

    /** Creates timeline from binary format. */
    void deserialize(IO::DataStream&);

    /** Stores the timeline to a binary file */
    void saveFile(const QString& filename);

    /** Reads a timeline from a binary file */
    void loadFile(const QString& filename);

    // ---------- get ----------------

    /** Comparison */
    bool operator == (const Timeline1d& other) const;

    /** return number of que points */
    unsigned int size() const { return data_.size(); }

    /** Returns wheter the timeline has no points */
    bool empty() const { return data_.empty(); }

    /** get value at time (with limits) */
    Double get(Double time) const;

    /** get value at time (without limits) */
    Double getNoLimit(Double time) const;

    /** get derivative at time (with limits).
        @p range is the range to observe for calculating the derivative
        and <b>must not</b> be smaller than timeQuantum(). */
    Double derivative(Double time, Double range = 0.01) const;

    /** return smallest time */
    Double tmin() const
    {
        if (data_.begin() == data_.end()) return 0.0;
        else return data_.begin()->second.t;
    }

    /** return largest time */
    Double tmax() const
    {
        if (data_.rbegin() == data_.rend()) return 0.0;
        else return data_.rbegin()->second.t;
    }

    /** return the reference on the data point structure */
    TpList &getData() { return data_; }
    const TpList &getData() const { return data_; }

    /** returns the TpList::iterator for a point at time 't',
        or data_.end() if there is no point */
    TpList::iterator find(Double t)
    {
        const TpHash h = hash(t);
        auto i = data_.find(h);
        // must check left and right because of rounding
        if (i==data_.end())
            i = data_.find(h+1);
        if (i==data_.end())
            i = data_.find(h-1);
        return i;
    }
    TpList::const_iterator find(Double t) const
    {
        const TpHash h = hash(t);
        auto i = data_.find(h);
        // must check left and right because of rounding
        if (i==data_.end())
            i = data_.find(h+1);
        if (i==data_.end())
            i = data_.find(h-1);
        return i;
    }

    /** return the TpList::iterator of the next point after time 't',
        or data_.end() if none there. <br>
        returns the next point <b>even</b> if there is a point at 't' */
    TpList::iterator next_after(Double t)
    {
        return data_.upper_bound(hash(t));
    }

    /** return the TpList::iterator of the first point >= time 't',
        or data_.end() if none there */
    TpList::iterator first(Double t)
    {
        return data_.lower_bound(hash(t));
    }

    /** return the closest point to time 't', or data_.end() */
    TpList::iterator closest(Double t);

    /** Returns the smallest and greatest values found within the given time range.
        Any limit is applied. Only the que points are considered, any curve
        exceeding the points is not considered. */
    void getMinMax(Double tStart, Double tEnd, Double& minimal, Double& maximal);

    // ---------- modify -------------

    /** remove all points */
    void clear();

    /** Adds the timeline data at the specified offset */
    void addTimeline(const Timeline1d&, Double timeOffset);

    /** Adds the timeline data at the specified offset and
        deletes any previous points in that area. */
    void overwriteTimeline(const Timeline1d&, Double timeOffset);

    /** adds a point if time is not already present */
    Point* add(Double time, Double value, TimelinePoint::Type typ = TimelinePoint::DEFAULT);

    /** adds a point if time is not already present and if the timeline at this point
        is not already 'value' +/- 'thresh' */
    Point* add(Double time, Double value, Double thresh, TimelinePoint::Type typ = TimelinePoint::DEFAULT);

    /** adds a point as given in structure */
    Point* add(Point &p);

    /** deletes a point if it is at time */
    void remove(Double time);

    /** removes the point with this hash. */
    void remove(TpHash hash);

    /** Removes all points in the given range */
    void remove(Double start, Double end);

    /** set a low and high limit for output values */
    void setLimit(Double lmin, Double lmax)
    {
        setLowerLimit(lmin);
        setUpperLimit(lmax);
    }

    /** set a lower limit for the output */
    void setLowerLimit(Double lmin)
    {
        lmin_ = lmin;
        lowerLimit_ = true;
    }

    /** set an upper limit for the output */
    void setUpperLimit(Double lmax)
    {
        lmax_ = lmax;
        upperLimit_ = true;
    }

    /** enable or disable lower limit */
    void enableLowerLimit(bool doLimit) { lowerLimit_ = doLimit; }
    /** enable or disable upper limit */
    void enableUpperLimit(bool doLimit) { upperLimit_ = doLimit; }

    /** automatically set the derivative for point 'i' */
    void setAutoDerivative(TpList::iterator &i);
    /** automatically set the derivatives for all points of set
        'start' to 'end' - 1 */
    void setAutoDerivative(TpList::iterator start, TpList::iterator end);
    /** automatically set the derivatives for all points */
    void setAutoDerivative() { setAutoDerivative(data_.begin(), data_.end()); }

    // ------------- functions- --------------

    /** shift all data-points by 'secOff' seconds */
    void shiftTime(Double secOff);

    /** scales all values to the range -amp to amp */
    void normalize(Double amp = 1.0);

    // ---------- more handling ----------------

    /** prints all point's values to 'out' */
    void print(std::ostream &out = std::cout)
    {
        for (TpList::iterator i=data_.begin(); i != data_.end(); i++)
            out << i->first << ":\t" << i->second.t << ", " << i->second.val << "\n";
    }


    private: // --------------- private area ---------------------

    // ------------- members -------------------

    /** all data points */
    TpList data_;

    /** current (last modified) point */
    Point *cur_;

    /** true if lower limit is used */
    bool lowerLimit_,
    /** true if upper limit is used */
         upperLimit_;

    Double
    /** the lower limit for output */
        lmin_,
    /** the upper limit for output */
        lmax_;

    // ------------------- private functions -----------------------

    /** type of point at this location, or default. */
    TimelinePoint::Type currentType_(Double time);

    /** returns true if the iterator 'i' is the LAST element in data */
    bool isLastElement_(TpList::iterator i) const
    {
        if (i==data_.end()) return false;
        i++;
        return (i==data_.end());
    }

    bool isLastElement_(TpList::const_iterator i) const
    {
        if (i==data_.end()) return false;
        i++;
        return (i==data_.end());
    }

    /** linearly fade between the two points. <br>
        'time' must be <b>between</b> 'p1's and 'p2's time */
    Double fadeLinear_(Point &p1, Point &p2, Double time) const
    {
        Double f = (time - p1.t) / (p2.t - p1.t);
        return p1.val*(1.0-f) + f*p2.val;
    }

    /** limit the value according to limit-settings */
    Double limit_(Double val) const
    {
        if (lowerLimit_) val = std::max(lmin_, val);
        if (upperLimit_) val = std::min(lmax_, val);
        return val;
    }

};


} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_TIMELINE1D_H
