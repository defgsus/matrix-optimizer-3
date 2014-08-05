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
template <typename F>
inline F linearstep(F edge0, F edge1, F x)
{
    return std::max((F)0, std::min((F)1, (x-edge0) / (edge1-edge0) ));
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
template <typename F>
inline F smootherstep(F edge0, F edge1, F x)
{
    x = linearstep(edge0, edge1, x);
    return x * x * x * (x * (x * (F)6 - (F)15) + (F)10);
}


/** Interpolate linearily between y and y1, by t [0,1] */
template <typename F>
inline F interpol_linear(F t, F y, F y1)
{
    return y + t * (y1 - y);
}

/** Interpolate smoothly between y and y1, by t [0,1] */
template <typename F>
inline F interpol_smooth(F t, F y, F y1)
{
    t = t * t * ((F)3 - (F)2 * t);
    return y + t * (y1 - y);
}

/** 6-point interpolation. <br>
    from somewhere in musicdsp.org <br>

    'ymx' = the xth left value, <br>
    'yx' = the xth right value. <br>
    't' must be between 0 and 1, describing the transition between 'y' and 'y1'
*/
template <typename F>
inline F interpol_6(F t, F ym2, F ym1, F y, F y1, F y2, F y3)
{
    return 	y + (F)0.04166666666*t*((y1-ym1)*(F)16.0+(ym2-y2)*(F)2.0
              + t *((y1+ym1)*(F)16.0-ym2-y*(F)30.0- y2
              + t *(y1*(F)66.0-y*(F)70.0-y2*(F)33.0+ym1*(F)39.0+ y3*(F)7.0- ym2*(F)9.0
              + t *(y*(F)126.0-y1*(F)124.0+y2*(F)61.0-ym1*(F)64.0- y3*(F)12.0+ym2*(F)13.0
              + t *((y1-y)*(F)50.0+(ym1-y2)*(F)25.0+(y3-ym2)*(F)5.0)))));
}

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_INTERPOL_H
