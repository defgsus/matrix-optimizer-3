/***************************************************************************

Copyright (C) 2014  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/
/** @file

    MAG::Timeline::Timeline1D component

    @author stefan.berke@modular-audio-graphics.com
    @version 2011/11/16 created (grabed together from kepler project)
    @version 2014/04/24 grabed from libmag
    */
#ifndef MOSRC_MATH_Timeline1D_H
#define MOSRC_MATH_Timeline1D_H

#include <iostream>
#include <list>
#include <map>
#include <vector>


namespace MO {


/**	a super-duper timeline component mapping seconds to values.

    <p>currently the hash value for the time component of a point is <b>(long int)(time * 4096)</b>.
    that means you can set 4096 different points per one-second-of-timeline and time can be maximally
    +/- 524287 seconds or ~8738 minutes or ~145 hours.</p>
*/
class Timeline1D
{
    public:

    // ------------ types -------------

    /** type of data is always double */
    typedef double Float;

    /** one que-point of a Timeline1D */
    struct Point
    {
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

        /** returns the name of the type.
            these names are */
        static const char *getName(Type type);

        /** returns the <b>persistent</b> name of the type.
            these names <b>must never change</b>!! */
        static const char *getPersistentName(Type type);

        Float
            /** time of the que-point in seconds */
            t,
            /** value of the que-point */
            val,
            /** first derrivative of the point, user-adjustable */
            d1;

        /** type of this que-point, see Timeline1D::Point::Type */
        Type type;
    };

    /** the type of hash-value generated for the time of a point. <br>
        <b>NOTE</b>: actually this is no random hash value. the points
        in TpList (more specific in Timeline1D::data) are always ordered
        according to their time. TpHash is simply the integer version of
        the time multiplied by a reasonable large constant. */
    typedef long int TpHash;

    /** the default container mapping between hashvalues and points */
    typedef std::map<TpHash, Point> TpList;


    // ------- ctor / dtor -----------

    /** default constructor, no parameters */
    Timeline1D();

    /** default destructor, wipes out everything */
    virtual ~Timeline1D();

    // ---------- get ----------------

    /** return number of que points */
    unsigned int size() const { return data_.size(); }

    /** get value at time (with limits) */
    Float get(Float time);

    /** get value at time (without limits) */
    Float getNoLimit(Float time);

    /** return smallest time */
    Float tmin() const {
        if (data_.begin() == data_.end()) return 0.0;
        else return data_.begin()->second.t;
    }

    /** return largest time */
    Float tmax() const {
        if (data_.rbegin() == data_.rend()) return 0.0;
        else return data_.rbegin()->second.t;
    }

    /** return the reference on the data point structure */
    TpList &getData() { return data_; }

    /** returns the TpList::iterator for a point at time 't',
        or data_.end() if there is no point */
    TpList::iterator find(Float t)
    {
        TpList::iterator i = data_.find(hash(t));
        // must check left and right because of rounding errors
        if (i==data_.end())
            i = data_.find(hash(t)+1);
        if (i==data_.end())
            i = data_.find(hash(t)-1);
        return i;
    }

    /** return the TpList::iterator of the next point after time 't',
        or data_.end() if none there. <br>
        returns the next point <b>even</b> if there is a point at 't' */
    TpList::iterator next_after(Float t)
    {
        return data_.upper_bound(hash(t));
    }

    /** return the TpList::iterator of the first point >= time 't',
        or data_.end() if none there */
    TpList::iterator first(Float t)
    {
        return data_.lower_bound(hash(t));
    }

    /** return the closest point to time 't', or data_.end() */
    TpList::iterator closest(Float t);

    // ---------- modify -------------

    /** remove all points */
    void clear();

    /** adds a point if time is not already present */
    Point* add(Float time, Float value, Point::Type typ = Point::DEFAULT);

    /** adds a point if time is not already present and if the timeline at this point
        is not already 'value' +/- 'thresh' */
    Point* add(Float time, Float value, Float thresh, Point::Type typ = Point::DEFAULT);

    /** adds a point as given in structure */
    Point* add(Point &p);

    /** deletes a point if it is at time */
    void remove(Float time);

    /** set a low and high limit for output values */
    void setLimit(Float lmin, Float lmax)
    {
        setLowerLimit(lmin);
        setUpperLimit(lmax);
    }

    /** set a lower limit for the output */
    void setLowerLimit(Float lmin)
    {
        lmin_ = lmin;
        lowerLimit_ = true;
    }

    /** set an upper limit for the output */
    void setUpperLimit(Float lmax)
    {
        lmax_ = lmax;
        upperLimit_ = true;
    }

    /** enable or disable lower limit */
    void setLowerLimit(bool doLimit) { lowerLimit_ = doLimit; }
    /** enable or disable upper limit */
    void setUpperLimit(bool doLimit) { upperLimit_ = doLimit; }

    /** automatically set the derivative for point 'i' */
    void setAutoDerivative(TpList::iterator &i);
    /** automatically set the derivatives for all points of set
        'start' to 'end' - 1 */
    void setAutoDerivative(TpList::iterator start, TpList::iterator end);
    /** automatically set the derivatives for all points */
    void setAutoDerivative() { setAutoDerivative(data_.begin(), data_.end()); }

    // ------------- functions- --------------

    /** shift all data-points by 'secOff' seconds */
    void shiftTime(Float secOff);

    /** scales all values to the range -amp to amp */
    void normalize(Float amp = 1.0);

    // ---------- more handling ----------------

    /** prints all point's values to 'out' */
    void print(std::ostream &out = std::cout)
    {
        for (TpList::iterator i=data_.begin(); i != data_.end(); i++)
            out << i->first << ":\t" << i->second.t << ", " << i->second.val << "\n";
    }

    /** stores the timeline to a text-file */
    bool saveFile(const char *filename);
    /** stores the timeline to a text-stream */
    void saveFile(FILE *f);

    /** reads a timeline from a text-file */
    bool loadFile(const char *filename);
    /** reads a timeline from a text-stream */
    bool loadFile(FILE *f);


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

    Float
    /** the lower limit for output */
        lmin_,
    /** the upper limit for output */
        lmax_;

    // ------------------- private functions -----------------------

    /** type of point at this location, or default. */
    Point::Type currentType_(Float time);

    /** returns true if the iterator 'i' is the LAST element in data */
    bool isLastElement_(TpList::iterator i)
    {
        if (i==data_.end()) return false;
        i++;
        return (i==data_.end());
    }

    /** linearly fade between the two points. <br>
        'time' must be <b>between</b> 'p1's and 'p2's time */
    Float fadeLinear_(Point &p1, Point &p2, Float time)
    {
        Float f = (time - p1.t) / (p2.t - p1.t);
        return p1.val*(1.0-f) + f*p2.val;
    }

    /** limit the value according to limit-settings */
    Float limit_(Float val)
    {
        if (lowerLimit_) val = std::max(lmin_, val);
        if (upperLimit_) val = std::min(lmax_, val);
        return val;
    }

    /** return the hash value for a specific time */
    TpHash hash(Float time)
    {
        return (TpHash)(time * 4096.0);
    }

};

} // namespace MO

#endif // MOSRC_MATH_Timeline1D_H
