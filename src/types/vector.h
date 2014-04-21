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

#ifndef MOSRC_TYPES_VECTOR_H
#define MOSRC_TYPES_VECTOR_H

#include <iostream>
#include <glm/glm.hpp>

// -------------------- universal constants ----------------

// XXX find better .h file

/** the infamous PI */
#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

/** 2.0 * PI */
#ifndef TWO_PI
#define TWO_PI (6.283185307179586476925286766559)
#endif

/** PI / 2.0 */
#ifndef HALF_PI
#define HALF_PI (1.5707963267948966192313216916398)
#endif

/** 2.0/3.0 * PI */
#ifndef TWOTHIRD_PI
#define TWOTHIRD_PI (2.0943951023931954923084289221863)
#endif

/** degree to 2*pi multiplier (TWO_PI/360.0) */
#ifndef DEG_TO_TWO_PI
#define DEG_TO_TWO_PI (PI/180.0)
#endif

/** 2*pi to degree multiplier (360.0/TWO_PI) */
#ifndef TWO_PI_TO_DEG
#define TWO_PI_TO_DEG (180.0/PI)
#endif

/** degree to radians multiplier (1.0/360.0) */
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (0.0027777777777777777777777777777)
#endif

/** sqrt(2.0) */
#ifndef SQRT_2
#define SQRT_2 (1.4142135623730950488016887242097)
#endif

/** tanh(1.0) */
#ifndef TANH_1
#define TANH_1 (0.761594)
#endif

/** 1.0/tanh(1.0) */
#ifndef TANH_1_I
#define TANH_1_I (1.313035)
#endif


namespace MO {

    // ------------ basic numeric types -------------

    typedef float Float;

    // -------- basic vector and matrix types -------

    typedef glm::detail::tvec2<Float> Vec2;
    typedef glm::detail::tvec3<Float> Vec3;
    typedef glm::detail::tvec4<Float> Vec4;

    typedef glm::detail::tmat3x3<Float> Mat3;
    typedef glm::detail::tmat4x4<Float> Mat4;

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

// ------------ iostream bindings ---------------

/** stream output of a vector */
template <typename F>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec2<F>& v)
{
    s << '<' << v[0] << ", " << v[1] << '>';
    return s;
}

/** stream output of a vector */
template <typename F>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec3<F>& v)
{
    s << '<' << v[0] << ", " << v[1] << ", " << v[2] << '>';
    return s;
}

/** stream output of a vector */
template <typename F>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec4<F>& v)
{
    s << '<' << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << '>';
    return s;
}

/** stream output of a 3by3 matrix */
template <typename F>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat3x3<F>& m)
{
    for (int i=0; i<3; ++i)
    {
        s << "<" << m[0][i] << ", " << m[1][i] << ", " << m[2][i] << ">" << std::endl;
    }
    return s;
}

/** stream output of a 4by4 matrix */
template <typename F>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat4x4<F>& m)
{
    for (int i=0; i<4; ++i)
    {
        s << "<";
        for (int j=0; j<3; ++j)
            s << m[j][i] << ", ";
        s << m[3][i] << ">" << std::endl;
    }
    return s;
}

#endif // MOSRC_TYPES_VECTOR_H
