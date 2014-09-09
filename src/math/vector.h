/** @file

    @brief useful functions for vectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/23</p>
*/

#ifndef MOSRC_MATH_VECTOR_H
#define MOSRC_MATH_VECTOR_H

#include "types/vector.h"
#include "math/constants.h"

namespace MO {
namespace MATH {

/** Returns a point on a unit sphere (radius = 1.0). <br>
    Given a point P = <0,1,0>, and v = rotation around z, and u = rotation around y <br>
    then the result is the rotated P. <br>
    u and v are in the range of 0..1 for the whole sphere */
    template <typename F>
    glm::detail::tvec3<F,glm::defaultp> pointOnSphere(F u, F v)
    {
        u *= TWO_PI,
        v *= PI;
        auto P = glm::detail::tvec3<F,glm::defaultp>(
        // rotate a point (0,1,0) around z
            -sin(v), std::cos(v), 0 );
        // rotate this point around y
        P.z = -P.x * std::sin(u);
        P.x =  P.x * std::cos(u);
        return P;
    }

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_VECTOR_H
