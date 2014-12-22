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

#if GLM_VERSION >= 95

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

    /** 2D rotation */
    template <typename T, glm::precision P>
    inline glm::detail::tvec2<T, P> rotate(const glm::detail::tvec2<T, P> & v, Float angle_degree)
    {
        const Float a = deg_to_rad(angle_degree);
        const Float s = std::sin(a),
                    c = std::cos(a);

        return Vec2(v.x * c - v.y * s,
                    v.x * s + v.y * c);
    }

    template <typename T, glm::precision P>
    inline glm::detail::tvec2<T, P> rotateZ(const glm::detail::tvec2<T, P> & v, Float angle_degree)
    {
        return rotate(v, angle_degree);
    }

    /** 3D rotation */
    template <typename T, glm::precision P>
    inline glm::detail::tvec3<T, P> rotate(const glm::detail::tvec3<T, P> & v,
                                           const glm::detail::tvec3<T, P> & axis, Float angle_degree)
    {
        const glm::detail::tvec4<T, P> v4 =
        #ifndef MO_GLM_RADIANS
            glm::rotate(Mat4(1), angle_degree, axis) * glm::detail::tvec4<T, P>(v, 1.0);
        #else
            glm::rotate(Mat4(1), deg_to_rad(angle_degree), axis) * glm::detail::tvec4<T, P>(v, 1.0);
        #endif
        return glm::detail::tvec3<T, P>(v4);
    }

    /** rotation of 4D vector */
    template <typename T, glm::precision P>
    inline glm::detail::tvec4<T, P> rotate(const glm::detail::tvec4<T, P> & v,
                                           const glm::detail::tvec3<T, P> & axis, Float angle_degree)
    {
        return
        #ifndef MO_GLM_RADIANS
            glm::rotate(Mat4(1), angle_degree, axis) * v;
        #else
            glm::rotate(Mat4(1), deg_to_rad(angle_degree), axis) * v;
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


    template <typename F, glm::precision P>
    inline glm::detail::tvec3<F, P> normalize_safe(
            const glm::detail::tvec3<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y + v.z * v.z;
        if (sqr > 0)
            return v / std::sqrt(sqr);
        else
            return v;
    }

    template <typename F, glm::precision P>
    inline glm::detail::tvec4<F, P> normalize_safe(
            const glm::detail::tvec4<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
        if (sqr > 0)
            return v / std::sqrt(sqr);
        else
            return v;
    }

#else // GLM_VERSION



template <typename T>
glm::detail::tmat4x4<T> rotate(glm::detail::tmat4x4<T> const & m,
                                T const & angle_degree,
                                glm::detail::tvec3<T> const & v)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotate(m, angle_degree, v);
    #else
        return glm::rotate(m, deg_to_rad(angle_degree), v);
    #endif
}

template <typename T>
glm::detail::tvec3<T> rotateX(glm::detail::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateX(v, angle_degree);
    #else
        return glm::rotateX(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
glm::detail::tvec3<T> rotateY(glm::detail::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateY(v, angle_degree);
    #else
        return glm::rotateY(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
glm::detail::tvec3<T> rotateZ(glm::detail::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateZ(v, angle_degree);
    #else
        return glm::rotateZ(v, deg_to_rad(angle_degree));
    #endif
}


template <typename T>
glm::detail::tvec4<T> rotateX(glm::detail::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateX(v, angle_degree);
    #else
        return glm::rotateX(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
glm::detail::tvec4<T> rotateY(glm::detail::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateY(v, angle_degree);
    #else
        return glm::rotateY(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
glm::detail::tvec4<T> rotateZ(glm::detail::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateZ(v, angle_degree);
    #else
        return glm::rotateZ(v, deg_to_rad(angle_degree));
    #endif
}


template <typename T>
glm::detail::tmat4x4<T> perspective(T const & fovy_degree,
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

/** 2D rotation */
template <typename T>
inline glm::detail::tvec2<T> rotate(const glm::detail::tvec2<T> & v, Float angle_degree)
{
    const Float a = deg_to_rad(angle_degree);
    const Float s = std::sin(a),
                c = std::cos(a);

    return Vec2(v.x * c - v.y * s,
                v.x * s + v.y * c);
}


/** Returns a point on a unit sphere (radius = 1.0). <br>
Given a point P = <0,1,0>, and v = rotation around z, and u = rotation around y <br>
then the result is the rotated P. <br>
u and v are in the range of 0..1 for the whole sphere */
template <typename F>
glm::detail::tvec3<F> pointOnSphere(F u, F v)
{
    u *= TWO_PI,
    v *= PI;
#if GLM_VERSION >= 95
    auto P = glm::detail::tvec3<F,glm::defaultp>(
#else
    auto P = glm::detail::tvec3<F>(
#endif
    // rotate a point (0,1,0) around z
        -sin(v), std::cos(v), 0 );
    // rotate this point around y
    P.z = -P.x * std::sin(u);
    P.x =  P.x * std::cos(u);
    return P;
}


template <typename F>
inline glm::detail::tvec3<F> normalize_safe(
        const glm::detail::tvec3<F>& v)
{
    F sqr = v.x * v.x + v.y * v.y + v.z * v.z;
    if (sqr > 0)
        return v / std::sqrt(sqr);
    else
        return v;
}





#endif



} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_VECTOR_H
