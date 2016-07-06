/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifndef MOSRC_MATH_TIMELINEND_H
#define MOSRC_MATH_TIMELINEND_H

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include <QString>

#include "types/float.h"
#include "types/refcounted.h"
#include "math/arithmeticarray.h"
#include "math/timeline_point.h"

namespace MO {
namespace IO { class DataStream; }
namespace MATH {

class Timeline1d;

/**	a super-duper timeline component mapping seconds to values.

    <p>currently the hash value for the time component of a point is <b>(long int)(time * 4096)</b>.
    that means you can set 4096 different points per one-second-of-timeline and time can be maximally
    +/- 524287 seconds or ~8738 minutes or ~145 hours.</p>
*/
class TimelineNd : public RefCounted
{
    public:

    // ------------ types -------------

    typedef ArithmeticArray<Double> ValueType;

    /** one que-point of a Timeline1D */
    struct Point
    {
        Point() : t(0), val(0), d1(0), type(TimelinePoint::CONSTANT) { }


        Double
            /** time of the que-point in seconds */
            t;
        ValueType
            /** value of the que-point */
            val,
            /** first derrivative of the point, user-adjustable */
            d1;

        /** type of this que-point, see Timeline1D::Point::Type */
        TimelinePoint::Type type;

        //bool operator == (const Point& o) const { return t == o.t && val == o.val && d1 == o.d1 && type == o.type; }
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

    static bool hasAutoDerivative(TimelinePoint::Type type);

    // ------- ctor / dtor -----------

    TimelineNd(size_t numDimensions);
    protected:    ~TimelineNd(); public:

    // ------ copy assignment --------

    TimelineNd(const TimelineNd& other);
    const TimelineNd& operator = (const TimelineNd& other);

    // ----------- io ----------------
#if 0
    /** Writes timeline to binary format. */
    void serialize(IO::DataStream&);

    /** Creates timeline from binary format. */
    void deserialize(IO::DataStream&);

    /** Stores the timeline to a binary file */
    void saveFile(const QString& filename);

    /** Reads a timeline from a binary file */
    void loadFile(const QString& filename);
#endif
    // ---------- get ----------------

    /* Comparison */
    //bool operator == (const TimelineNd& other) const;

    /** return number of que points */
    size_t size() const { return p_data_.size(); }

    size_t numDimensions() const { return p_dim_; }

    /** Returns wheter the timeline has no points */
    bool empty() const { return p_data_.empty(); }

    /** get value at time (with limits) */
    ValueType get(Double time) const;

    /** get value at time (without limits) */
    ValueType getNoLimit(Double time) const;

    /** get derivative at time (with limits).
        @p range is the range to observe for calculating the derivative
        and <b>must not</b> be smaller than timeQuantum(). */
    ValueType getDerivative(Double time, Double range = 0.01) const;

    /** return smallest time */
    Double tmin() const
    {
        if (p_data_.begin() == p_data_.end()) return 0.0;
        else return p_data_.begin()->second.t;
    }

    /** return largest time */
    Double tmax() const
    {
        if (p_data_.rbegin() == p_data_.rend()) return 0.0;
        else return p_data_.rbegin()->second.t;
    }

    /** return the reference on the data point structure */
    TpList &getData() { return p_data_; }
    const TpList &getData() const { return p_data_; }

    /** returns the TpList::iterator for a point at time 't',
        or data_.end() if there is no point */
    TpList::iterator find(Double t)
    {
        const TpHash h = hash(t);
        auto i = p_data_.find(h);
        // must check left and right because of rounding
        if (i==p_data_.end())
            i = p_data_.find(h+1);
        if (i==p_data_.end())
            i = p_data_.find(h-1);
        return i;
    }
    TpList::const_iterator find(Double t) const
    {
        const TpHash h = hash(t);
        auto i = p_data_.find(h);
        // must check left and right because of rounding
        if (i==p_data_.end())
            i = p_data_.find(h+1);
        if (i==p_data_.end())
            i = p_data_.find(h-1);
        return i;
    }

    /** return the TpList::iterator of the next point after time 't',
        or data_.end() if none there. <br>
        returns the next point <b>even</b> if there is a point at 't' */
    TpList::iterator next_after(Double t)
    {
        return p_data_.upper_bound(hash(t));
    }

    /** return the TpList::iterator of the first point >= time 't',
        or data_.end() if none there */
    TpList::iterator first(Double t)
    {
        return p_data_.lower_bound(hash(t));
    }

    /** return the closest point to time 't', or data_.end() */
    TpList::iterator closest(Double t);

    /** Returns the smallest and greatest values found within the given time range.
        Any limit is applied. Only the que points are considered, any curve
        exceeding the points is not considered. */
    void getMinMax(Double tStart, Double tEnd, ValueType& minimal, ValueType& maximal);

    /** Returns a 1d copy of the given dimension */
    Timeline1d* getTimeline1d(size_t dimension = 0);

    /** Returns a Xd copy of the given dimensions */
    TimelineNd* getTimelineNd(size_t first, size_t num = 1);

    // ---------- modify -------------

    /** remove all points */
    void clear();

    /** Changes the dimensionality of the whole Timeline. */
    void setDimensions(size_t newDim);

#if 0
    /** Adds the timeline data at the specified offset */
    void addTimeline(const Timeline1d&, Double timeOffset);

    /** Adds the timeline data at the specified offset and
        deletes any previous points in that area. */
    void overwriteTimeline(const Timeline1d&, Double timeOffset);
#endif

    /** adds a point if time is not already present */
    Point* add(Double time, const ValueType& value,
               TimelinePoint::Type typ = TimelinePoint::DEFAULT);
#if 0
    /** adds a point if time is not already present and if the timeline at this point
        is not already 'value' +/- 'thresh' */
    Point* add(Double time, Double value, Double thresh, Point::Type typ = Point::DEFAULT);
#endif
    /** adds a point as given in structure */
    Point* add(Point &p);

    /** deletes a point if it is at time */
    void remove(Double time);

    /** removes the point with this hash. */
    void remove(TpHash hash);

    /** Removes all points in the given range */
    void remove(Double start, Double end);

    /** set a low and high limit for output values */
    void setLimit(const ValueType lmin, const ValueType lmax)
    {
        setLowerLimit(lmin);
        setUpperLimit(lmax);
    }

    /** set a lower limit for the output */
    void setLowerLimit(const ValueType& lmin)
    {
        MO_AA_ASSERT(lmin.numDimensions() == p_dim_);
        p_minVal_ = lmin;
        p_isLowerLimit_ = true;
    }

    /** set an upper limit for the output */
    void setUpperLimit(const ValueType& lmax)
    {
        MO_AA_ASSERT(lmax.numDimensions() == p_dim_);
        p_maxVal_ = lmax;
        p_isUpperLimit_ = true;
    }

    /** enable or disable lower limit */
    void enableLowerLimit(bool doLimit) { p_isLowerLimit_ = doLimit; }
    /** enable or disable upper limit */
    void enableUpperLimit(bool doLimit) { p_isUpperLimit_ = doLimit; }

    /** automatically set the derivative for point 'i' */
    void setAutoDerivative(TpList::iterator &i);
    /** automatically set the derivatives for all points of set
        'start' to 'end' - 1 */
    void setAutoDerivative(TpList::iterator start, TpList::iterator end);
    /** automatically set the derivatives for all points */
    void setAutoDerivative() { setAutoDerivative(p_data_.begin(), p_data_.end()); }

    // ------------- functions- --------------

    /** shift all data-points by 'secOff' seconds */
    void shiftTime(Double secOff);

    /* scales all values to the range -amp to amp */
    //void normalize(Double amp = 1.0);

    // ---------- more handling ----------------

    /** prints all point's values to 'out' */
    void print(std::ostream &out = std::cout);
    /*
    {
        for (TpList::iterator i=data_.begin(); i != data_.end(); i++)
            out << i->first << ":\t" << i->second.t << ", " << i->second.val << "\n";
    }*/


    private: // --------------- private area ---------------------

    // ------------- members -------------------


    /** all data points */
    TpList p_data_;

    size_t p_dim_;

    /** current (last modified) point */
    Point *p_cur_;

    /** true if lower limit is used */
    bool p_isLowerLimit_,
    /** true if upper limit is used */
         p_isUpperLimit_;

    ValueType
    /** the lower limit for output */
        p_minVal_,
    /** the upper limit for output */
        p_maxVal_;

    // ------------------- private functions -----------------------

    /** type of point at this location, or default. */
    TimelinePoint::Type p_currentType_(Double time);

    /** returns true if the iterator 'i' is the LAST element in data */
    bool p_isLastElement_(TpList::iterator i) const
    {
        if (i==p_data_.end()) return false;
        i++;
        return (i==p_data_.end());
    }

    bool p_isLastElement_(TpList::const_iterator i) const
    {
        if (i==p_data_.end()) return false;
        i++;
        return (i==p_data_.end());
    }

    /** linearly fade between the two points. <br>
        'time' must be <b>between</b> 'p1's and 'p2's time */
    ValueType p_fadeLinear_(Point &p1, Point &p2, Double time) const
    {
        Double f = (time - p1.t) / (p2.t - p1.t);
        return p1.val*(1.0-f) + f*p2.val;
    }

    /** limit the value according to limit-settings */
    void p_limit_(ValueType& val) const
    {
        if (p_isLowerLimit_) val = max(p_minVal_, val);
        if (p_isUpperLimit_) val = min(p_maxVal_, val);
    }

};

inline bool TimelineNd::hasAutoDerivative(TimelinePoint::Type type)
{
    return (type == TimelinePoint::SYMMETRIC ||
            type == TimelinePoint::HERMITE);
}


} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_TIMELINEND_H
