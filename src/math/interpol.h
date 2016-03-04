/** @file

    @brief (templated) interpolation functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2012/03/02 and updated 2014/04/24</p>

    <p>credits: mathematicians and musicdsp.org fans worldwide</p>
*/
#ifndef MOSRC_MATH_INTERPOL_H
#define MOSRC_MATH_INTERPOL_H

namespace MO {
namespace MATH {

/** Put the value of @p x between [0,1]. <br/>
    a linear fade between 0 and 1 happens for @p x in the range [@p edge0,@p egde1]. <br/>
    edge0 and egde1 must not be equal! */
template <typename T, typename F>
inline F linearstep(F edge0, F edge1, T x)
{
    return std::max((T)0, std::min((T)1, (x-edge0) / (edge1-edge0) ));
}

/** Put the value of @p x between [0,1]. <br/>
    a smooth fade between 0 and 1 happens for @p x in the range [@p edge0,@p egde1]. <br/>
    edge0 and egde1 must not be equal! */
template <typename F>
inline F smoothstep(F edge0, F edge1, F x)
{
    x = linearstep(edge0, edge1, x);
    return x * x * ((F)3 - (F)2 * x);
}

/** Put the value of @p x between [0,1]. <br/>
    a smooth fade between 0 and 1 happens for @p x in the range [@p edge0,@p egde1]. <br/>
    edge0 and egde1 must not be equal! */
template <typename T, typename F>
inline F smootherstep(F edge0, F edge1, T x)
{
    x = linearstep(edge0, edge1, x);
    return x * x * x * (x * (x * (T)6 - (T)15) + (T)10);
}


/** Interpolate linearily between y and y1, by t [0,1] */
template <typename T, typename F>
inline F interpol_linear(T t, F y, F y1)
{
    return y + t * (y1 - y);
}

/** Interpolate smoothly between y and y1, by t [0,1] */
template <typename T, typename F>
inline F interpol_smooth(T t, F y, F y1)
{
    t = t * t * ((T)3 - (T)2 * t);
    return y + t * (y1 - y);
}

/** Interpolate smoothly (and steeper as interpol_smooth()) between y and y1, by t [0,1] */
template <typename T, typename F>
inline F interpol_smooth2(T t, F y, F y1)
{
    t = t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
    return y + t * (y1 - y);
}

/** 6-point interpolation. <br>
    from somewhere in musicdsp.org <br>

    'ymx' = the xth left value, <br>
    'yx' = the xth right value. <br>
    't' must be between 0 and 1, describing the transition between 'y' and 'y1'
*/
template <typename T, typename F>
inline F interpol_6(T t, F ym2, F ym1, F y, F y1, F y2, F y3)
{
    return 	y + (T)0.04166666666*t*((y1-ym1)*(T)16.0+(ym2-y2)*(T)2.0
              + t *((y1+ym1)*(T)16.0-ym2-y*(T)30.0- y2
              + t *(y1*(T)66.0-y*(T)70.0-y2*(T)33.0+ym1*(T)39.0+ y3*(T)7.0- ym2*(T)9.0
              + t *(y*(T)126.0-y1*(T)124.0+y2*(T)61.0-ym1*(T)64.0- y3*(T)12.0+ym2*(T)13.0
              + t *((y1-y)*(T)50.0+(ym1-y2)*(T)25.0+(y3-ym2)*(T)5.0)))));
}

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_INTERPOL_H
