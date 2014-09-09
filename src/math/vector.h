/** @file

    @brief useful functions for vectors

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/23</p>
*/

#ifndef MOSRC_MATH_VECTOR_H
#define MOSRC_MATH_VECTOR_H

#include <glm/gtx/rotate_vector.hpp>

#include "types/vector.h"
#include "math/constants.h"


#if (GLM_VERSION >= 95) && defined(GLM_FORCE_RADIANS)
#   define MO_GLM_RADIANS
#endif


namespace MO {
namespace MATH {

    // ------- wrapper for glm functions that take an angle ---------

    template <typename T, glm::precision P>
    glm::detail::tmat4x4<T, P> rotate(glm::detail::tmat4x4<T, P> const & m,
                                    T const & angle_degree,
                                    glm::detail::tvec3<T, P> const & v)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotate(m, angle_degree, v);
        #else
            return glm::rotate(m, deg_to_rad(angle_degree), v);
        #endif
    }

    template <typename T, glm::precision P>
    glm::detail::tvec3<T, P> rotateX(glm::detail::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateX(v, angle_degree);
        #else
            return glm::rotateX(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    glm::detail::tvec3<T, P> rotateY(glm::detail::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateY(v, angle_degree);
        #else
            return glm::rotateY(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    glm::detail::tvec3<T, P> rotateZ(glm::detail::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateZ(v, angle_degree);
        #else
            return glm::rotateZ(v, deg_to_rad(angle_degree));
        #endif
    }


    template <typename T, glm::precision P>
    glm::detail::tvec4<T, P> rotateX(glm::detail::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateX(v, angle_degree);
        #else
            return glm::rotateX(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    glm::detail::tvec4<T, P> rotateY(glm::detail::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateY(v, angle_degree);
        #else
            return glm::rotateY(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    glm::detail::tvec4<T, P> rotateZ(glm::detail::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateZ(v, angle_degree);
        #else
            return glm::rotateZ(v, deg_to_rad(angle_degree));
        #endif
    }


    template <typename T>
    glm::detail::tmat4x4<T, glm::defaultp> perspective(T const & fovy_degree,
                                                       T const & aspect,
                                                       T const & zNear,
                                                       T const & zFar)
    {
        #ifndef MO_GLM_RADIANS
            return glm::perspective(fovy_degree, aspect, zNear, zFar);
        #else
            return glm::perspective(deg_to_rad(fovy_degree), aspect, zNear, zFar);
        #endif
    }



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
