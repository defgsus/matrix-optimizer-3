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

namespace MO {
namespace IO { class DataStream; }
namespace MATH {

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
        Point() : t(0), val(0), d1(0), type(CONSTANT) { }

        /** possible types of points / interpolations */
        enum Type
        {
            /** default is used only to signal to use the last type. <br>
                it's no legal type to calculate with. */
            DEFAULT,
            /** the value will stay the same over time until
                the next que-point */
            CONSTANT,
            /** the value will linearly fade between this and the next point */
            LINEAR,
            /** the value will nicely fade between this and the next point in a sigmoid fashion */
            SMOOTH,
            /** the value will be interpolated between user-adjustable symmetric derrivatives at each point. <br>
                curve continuity at each point is granted. */
            SYMMETRIC,
            /** the value will be interpolated between user-adjustable symmetric derrivatives at each point. <br>
                variation of hermite interpolation. */
            SYMMETRIC2,
            /** 4-point spline.
                curve continuity at each point is granted. */
            SPLINE4_SYM,
            /** the value will describe the way of a nice spline (4 points are used) */
            SPLINE4,
            /** the value will describe the way of a very nice spline (6 points are used) */
            SPLINE6,
            /** this is no legal type, only the number of possible values */
            MAX
        };

        /** returns the user-friendly name of the type. */
        static const char *getName(Type type);

        /** returns the <b>persistent</b> name of the type.
            these names <b>must never change</b>!! */
        static const char *getPersistentName(Type type);

        /** Returns the type for the given persistent name,
            or LINEAR if unknown. */
        static Type getTypeForPersistentName(const QString& persistent_name);

        Double
            /** time of the que-point in seconds */
            t;
        ValueType
            /** value of the que-point */
            val,
            /** first derrivative of the point, user-adjustable */
            d1;

        /** type of this que-point, see Timeline1D::Point::Type */
        Type type;

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

    static bool hasAutoDerivative(Point::Type type);

    // ------- ctor / dtor -----------

    TimelineNd(size_t numDimensions);
    ~TimelineNd();

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
    size_t size() const { return data_.size(); }

    size_t numDimensions() const { return p_dim_; }

    /** Returns wheter the timeline has no points */
    bool empty() const { return data_.empty(); }

    /** get value at time (with limits) */
    ValueType get(Double time) const;

    /** get value at time (without limits) */
    ValueType getNoLimit(Double time) const;

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
    void getMinMax(Double tStart, Double tEnd, ValueType& minimal, ValueType& maximal);

    // ---------- modify -------------

    /** remove all points */
    void clear();

#if 0
    /** Adds the timeline data at the specified offset */
    void addTimeline(const Timeline1d&, Double timeOffset);

    /** Adds the timeline data at the specified offset and
        deletes any previous points in that area. */
    void overwriteTimeline(const Timeline1d&, Double timeOffset);
#endif

    /** adds a point if time is not already present */
    Point* add(Double time, const ValueType& value, Point::Type typ = Point::DEFAULT);
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
        lmin_ = lmin;
        lowerLimit_ = true;
    }

    /** set an upper limit for the output */
    void setUpperLimit(const ValueType& lmax)
    {
        MO_AA_ASSERT(lmax.numDimensions() == p_dim_);
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
    TpList data_;

    size_t p_dim_;

    /** current (last modified) point */
    Point *cur_;

    /** true if lower limit is used */
    bool lowerLimit_,
    /** true if upper limit is used */
         upperLimit_;

    ValueType
    /** the lower limit for output */
        lmin_,
    /** the upper limit for output */
        lmax_;

    // ------------------- private functions -----------------------

    /** type of point at this location, or default. */
    Point::Type currentType_(Double time);

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
    ValueType fadeLinear_(Point &p1, Point &p2, Double time) const
    {
        Double f = (time - p1.t) / (p2.t - p1.t);
        return p1.val*(1.0-f) + f*p2.val;
    }

    /** limit the value according to limit-settings */
    ValueType limit_(const ValueType& val) const
    {
        //if (lowerLimit_) val = std::max(lmin_, val);
        //if (upperLimit_) val = std::min(lmax_, val);
        return val;
    }

};

inline bool TimelineNd::hasAutoDerivative(Point::Type type)
{
    return (type == Point::SYMMETRIC ||
            type == Point::SYMMETRIC2);
}


} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_TIMELINEND_H
