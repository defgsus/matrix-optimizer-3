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

    useful functions for vectors
*/
#ifndef MOSRC_MATH_VECTOR_H
#define MOSRC_MATH_VECTOR_H

#include "types/vector.h"
#include "math/constants.h"

namespace MO {

/** Returns a point on a unit sphere (radius = 1.0). <br>
    Given a point P = <0,1,0>, and v = rotation around z, and u = rotation around y <br>
    then the result is the rotated P. <br>
    u and v are in the range of 0..1 for the whole sphere */
    template <typename F>
    glm::detail::tvec3<F> pointOnSphere(F u, F v)
    {
        u *= TWO_PI,
        v *= PI;
        auto P = glm::detail::tvec3<F>(
        // rotate a point (0,1,0) around z
            -sin(v), std::cos(v), 0 );
        // rotate this point around y
        P.z = -P.x * std::sin(u);
        P.x =  P.x * std::cos(u);
        return P;
    }


} // namespace MO

#endif // MOSRC_MATH_VECTOR_H
