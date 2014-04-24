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

#ifndef MOSRC_MATH_INTERPOL_H
#define MOSRC_MATH_INTERPOL_H

namespace MO {

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

} // namespace MO

#endif // MOSRC_MATH_INTERPOL_H
