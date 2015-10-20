/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#ifndef MOSRC_TOOL_VALUESMOOTHER_H
#define MOSRC_TOOL_VALUESMOOTHER_H

#include "math/interpol.h"
#include "math/interpolationtype.h"

namespace MO {


/** Smoother for stepped, timed float values.
    The method is almost deterministic, e.g.
    get() returns on-the-fly interpolation of input
    values. */
template <typename F>
class ValueSmoother
{
public:

    ValueSmoother()
        : p_timeIn      (F(0))
        , p_value       (F(0))
        , p_valueOut    (F(0))
        , p_valueNew    (F(0))
    { }

    /** Simply resets everything to zero */
    void reset();

    /** Sets a new goal value with given input time */
    void set(F in_time, F value);

    /** Retrieves the value at a given output time.
        @p smoothTime is the time it takes for the value to reach
        the new value given with set(). @p smoothTime <b>must</b> be larger 0.0 ! */
    F get(F out_time, F smoothTime = F(1), MATH::InterpolationType = MATH::IT_LINEAR);

    /** Returns non-smoothed last value from set() */
    F get() const { return p_valueNew; }

private:
    F p_timeIn
    , p_value
    , p_valueOut
    , p_valueNew;
};


// ------------ templ. impl. -----------------

template <typename F>
void ValueSmoother<F>::reset()
{
    p_timeIn = p_value = p_valueOut = p_valueNew = F(0);
}

template <typename F>
void ValueSmoother<F>::set(F time, F value)
{
    p_timeIn = time;
    p_valueNew = value;
    p_value = p_valueOut;
}

template <typename F>
F ValueSmoother<F>::get(F time, F st, MATH::InterpolationType it)
{
    // skip smoothing
    if (it == MATH::IT_NONE)
        return p_valueOut = p_valueNew;

    // before last input
    if (time < p_timeIn)
        return p_valueOut = p_value;
    // after smooth time
    if (time > p_timeIn + st)
        return p_valueOut = p_valueNew;
    // interpolate
    const F t = (time - p_timeIn) / st;

    if (it == MATH::IT_SMOOTH)
        return p_valueOut = MATH::interpol_smooth(t, p_value, p_valueNew);
    else if (it == MATH::IT_SMOOTH)
        return p_valueOut = MATH::interpol_smooth2(t, p_value, p_valueNew);
    else
        return p_valueOut = MATH::interpol_linear(t, p_value, p_valueNew);
}

} // namespace MO

#endif // MOSRC_TOOL_VALUESMOOTHER_H

