/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifndef MOSRC_MATH_VEC_MATH_H
#define MOSRC_MATH_VEC_MATH_H

#include <cmath>

namespace MO {
namespace MATH {

#define X 0
#define Y 1
#define Z 2
#define W 3

namespace INPLACE {


    /** v1 += v2 */
    template <typename F>
    void vec_add_n_n(F* v1, const F* v2, int n)
    {
        for (int i=0; i<n; ++i)
            v1[i] += v2[i];
    }

    /** v1 -= v2 */
    template <typename F>
    void vec_sub_n_n(F* v1, const F* v2, int n)
    {
        for (int i=0; i<n; ++i)
            v1[i] -= v2[i];
    }

    /** v1 /= v2 */
    template <typename F>
    void vec_div_n_n(F* v1, const F* v2, int n)
    {
        for (int i=0; i<n; ++i)
            v1[i] /= v2[i];
    }

    /** v1 *= v2 */
    template <typename F>
    void vec_mul_n_n(F* v1, const F* v2, int n)
    {
        for (int i=0; i<n; ++i)
            v1[i] *= v2[i];
    }

    /** normalize v */
    template <typename F>
    void vec_normalize_n(F* v, int n)
    {
        F mag = F(0);
        for (int i=0; i<n; ++i)
            mag += v[i] * v[i];
        if (mag > F(0))
        {
            mag = std::sqrt(mag);
            for (int i=0; i<n; ++i)
                v[i] /= mag;
        }
    }

    /** dst = cross(dst, src) */
    template <typename F>
    void vec_cross_3_3(F* dst, const F* src)
    {
        F x =    dst[Y] * src[Z] - dst[Z] * src[Y];
        F y =    dst[Z] * src[X] - dst[X] * src[Z];
        dst[Z] = dst[X] * src[Y] - dst[Y] * src[X];
        dst[Y] = y;
        dst[X] = x;
    }

    /** return perpendicular vector to a plane that
        crosses 0,0,0 and that v1 and v2 lay on. */
    template <typename F>
    void vec_ortho_3_3(F* dst, const F* src)
    {
        vec_cross_3(dst, src);
    }

    /** return perpendicular vector to a plane that
        v1, v2 and v3 lay on. */
    template <typename F>
    void vec_ortho_3_3_3(F* v1, const F* v2, const F* v3)
    {
        // cross(v2-v1, v3-v1)
        vec_sub_n_n(v2, v1, 3);
        vec_sub_n_n(v3, v1, 3);
        vec_cross_3(v2, v3);
    }

    /** return perpendicular vector to 0 -> v */
    template <typename F>
    void vec_ortho_2(F* v)
    {
        std::swap(v[X], v[Y]);
        v[X] = -v[X];
    }

    /** return perpendicular, normalized vector to v1 -> v2 */
    template <typename F>
    void vec_ortho_2_2(F* v1, const F* v2)
    {
        F x   = -(v2[Y] - v1[Y]);
        v1[Y] =   v2[X] - v1[X];
        v1[X] = x;
    }

} // namespace INPLACE

#undef X
#undef Y
#undef Z
#undef W

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_VEC_MATH_H

