/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#ifndef MOSRC_TOOL_LINEARIZERFLOAT_H
#define MOSRC_TOOL_LINEARIZERFLOAT_H

#include "types/float.h"

namespace MO {

/** A threadsafe timed float input with output interpolation */
class LinearizerFloat
{
public:

    enum InterpolationMode
    {
        IM_NONE,
        IM_LINEAR,
        IM_SMOOTH,
        IM_SMOOTH2
    };

    LinearizerFloat();
    ~LinearizerFloat();

    /** Reads a value from history.
        Threadsafe. */
    Double getValue(Double time) const;

    InterpolationMode interpolationMode() const;

    // ---- setter ----

    /** Clears all values.
        Threadsafe. */
    void clear();

    /** Puts a value into the stack.
        @p time must be increasing. Time values smaller or equal to the last
        time given to this function are silently ignored.
        Threadsafe. */
    void insertValue(Double time, Double value);

    void setInterpolationMode(InterpolationMode);

private:

    LinearizerFloat(const LinearizerFloat&) = delete;
    void operator=(const LinearizerFloat&) = delete;

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_TOOL_LINEARIZERFLOAT_H
