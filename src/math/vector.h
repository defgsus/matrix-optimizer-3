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



#if GLM_VERSION >= 95

    /** Returns the area of a triangle given by three corners */
    template <typename T, glm::precision P>
    T triangle_area(const MO_GLM_DETAIL::tvec3<T, P>& v1,
                    const MO_GLM_DETAIL::tvec3<T, P>& v2,
                    const MO_GLM_DETAIL::tvec3<T, P>& v3)
    {
        const T edge1 = glm::distance(v1, v2),
                edge2 = glm::distance(v2, v3),
                edge3 = glm::distance(v3, v1),
                s = (edge1 + edge2 + edge3) * T(0.5);
        return std::sqrt( s * (s-edge1) * (s-edge2) * (s-edge3) );
    }


    // ------- wrapper for glm functions that take an angle ---------

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tmat4x4<T, P> rotate(MO_GLM_DETAIL::tmat4x4<T, P> const & m,
                                    T const & angle_degree,
                                    MO_GLM_DETAIL::tvec3<T, P> const & v)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotate(m, angle_degree, v);
        #else
            return glm::rotate(m, deg_to_rad(angle_degree), v);
        #endif
    }

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec3<T, P> rotateX(MO_GLM_DETAIL::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateX(v, angle_degree);
        #else
            return glm::rotateX(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec3<T, P> rotateY(MO_GLM_DETAIL::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateY(v, angle_degree);
        #else
            return glm::rotateY(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec3<T, P> rotateZ(MO_GLM_DETAIL::tvec3<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateZ(v, angle_degree);
        #else
            return glm::rotateZ(v, deg_to_rad(angle_degree));
        #endif
    }


    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec4<T, P> rotateX(MO_GLM_DETAIL::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateX(v, angle_degree);
        #else
            return glm::rotateX(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec4<T, P> rotateY(MO_GLM_DETAIL::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateY(v, angle_degree);
        #else
            return glm::rotateY(v, deg_to_rad(angle_degree));
        #endif
    }

    template <typename T, glm::precision P>
    MO_GLM_DETAIL::tvec4<T, P> rotateZ(MO_GLM_DETAIL::tvec4<T, P> const & v,
                                    T const & angle_degree)
    {
        #ifndef MO_GLM_RADIANS
            return glm::rotateZ(v, angle_degree);
        #else
            return glm::rotateZ(v, deg_to_rad(angle_degree));
        #endif
    }


    template <typename T>
    MO_GLM_DETAIL::tmat4x4<T, glm::defaultp> perspective(T const & fovy_degree,
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
    inline MO_GLM_DETAIL::tvec2<T, P> rotate(const MO_GLM_DETAIL::tvec2<T, P> & v, Float angle_degree)
    {
        const Float a = deg_to_rad(angle_degree);
        const Float s = std::sin(a),
                    c = std::cos(a);

        return Vec2(v.x * c - v.y * s,
                    v.x * s + v.y * c);
    }

    template <typename T, glm::precision P>
    inline MO_GLM_DETAIL::tvec2<T, P> rotateZ(const MO_GLM_DETAIL::tvec2<T, P> & v, Float angle_degree)
    {
        return MO::MATH::rotate(v, angle_degree);
    }

    /** 3D rotation */
    template <typename T, glm::precision P>
    inline MO_GLM_DETAIL::tvec3<T, P> rotate(const MO_GLM_DETAIL::tvec3<T, P> & v,
                                           const MO_GLM_DETAIL::tvec3<T, P> & axis, Float angle_degree)
    {
        const MO_GLM_DETAIL::tvec4<T, P> v4 =
        #ifndef MO_GLM_RADIANS
            glm::rotate(Mat4(1), angle_degree, axis) * MO_GLM_DETAIL::tvec4<T, P>(v, 1.0);
        #else
            glm::rotate(Mat4(1), deg_to_rad(angle_degree), axis) * MO_GLM_DETAIL::tvec4<T, P>(v, 1.0);
        #endif
        return MO_GLM_DETAIL::tvec3<T, P>(v4);
    }

    /** rotation of 4D vector */
    template <typename T, glm::precision P>
    inline MO_GLM_DETAIL::tvec4<T, P> rotate(const MO_GLM_DETAIL::tvec4<T, P> & v,
                                           const MO_GLM_DETAIL::tvec3<T, P> & axis, Float angle_degree)
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
    MO_GLM_DETAIL::tvec3<F,glm::defaultp> pointOnSphere(F u, F v)
    {
        u *= TWO_PI,
        v *= PI;
        auto P = MO_GLM_DETAIL::tvec3<F,glm::defaultp>(
        // rotate a point (0,1,0) around z
            -sin(v), std::cos(v), 0 );
        // rotate this point around y
        P.z = -P.x * std::sin(u);
        P.x =  P.x * std::cos(u);
        return P;
    }


    template <typename F, glm::precision P>
    inline MO_GLM_DETAIL::tvec2<F, P> normalize_safe(
            const MO_GLM_DETAIL::tvec2<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y;
        if (sqr > 0)
            return v / std::sqrt(sqr);
        else
            return v;
    }

    template <typename F, glm::precision P>
    inline MO_GLM_DETAIL::tvec3<F, P> normalize_safe(
            const MO_GLM_DETAIL::tvec3<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y + v.z * v.z;
        if (sqr > 0)
            return v / std::sqrt(sqr);
        else
            return v;
    }

    template <typename F, glm::precision P>
    inline MO_GLM_DETAIL::tvec4<F, P> normalize_safe(
            const MO_GLM_DETAIL::tvec4<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
        if (sqr > 0)
            return v / std::sqrt(sqr);
        else
            return v;
    }


    template <typename F, glm::precision P>
    inline F length_safe(
            const MO_GLM_DETAIL::tvec2<F, P>& v)
    {
        F sqr = v.x * v.x + v.y * v.y;
        return (sqr > 0)
            ? std::sqrt(sqr) : 0.;
    }

#else // GLM_VERSION



template <typename T>
MO_GLM_DETAIL::tmat4x4<T> rotate(MO_GLM_DETAIL::tmat4x4<T> const & m,
                                T const & angle_degree,
                                MO_GLM_DETAIL::tvec3<T> const & v)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotate(m, angle_degree, v);
    #else
        return glm::rotate(m, deg_to_rad(angle_degree), v);
    #endif
}

template <typename T>
MO_GLM_DETAIL::tvec3<T> rotateX(MO_GLM_DETAIL::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateX(v, angle_degree);
    #else
        return glm::rotateX(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
MO_GLM_DETAIL::tvec3<T> rotateY(MO_GLM_DETAIL::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateY(v, angle_degree);
    #else
        return glm::rotateY(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
MO_GLM_DETAIL::tvec3<T> rotateZ(MO_GLM_DETAIL::tvec3<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateZ(v, angle_degree);
    #else
        return glm::rotateZ(v, deg_to_rad(angle_degree));
    #endif
}


template <typename T>
MO_GLM_DETAIL::tvec4<T> rotateX(MO_GLM_DETAIL::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateX(v, angle_degree);
    #else
        return glm::rotateX(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
MO_GLM_DETAIL::tvec4<T> rotateY(MO_GLM_DETAIL::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateY(v, angle_degree);
    #else
        return glm::rotateY(v, deg_to_rad(angle_degree));
    #endif
}

template <typename T>
MO_GLM_DETAIL::tvec4<T> rotateZ(MO_GLM_DETAIL::tvec4<T> const & v,
                                T const & angle_degree)
{
    #ifndef MO_GLM_RADIANS
        return glm::rotateZ(v, angle_degree);
    #else
        return glm::rotateZ(v, deg_to_rad(angle_degree));
    #endif
}


template <typename T>
MO_GLM_DETAIL::tmat4x4<T> perspective(T const & fovy_degree,
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
inline MO_GLM_DETAIL::tvec2<T> rotate(const MO_GLM_DETAIL::tvec2<T> & v, Float angle_degree)
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
MO_GLM_DETAIL::tvec3<F> pointOnSphere(F u, F v)
{
    u *= TWO_PI,
    v *= PI;
    auto P = MO_GLM_DETAIL::tvec3<F>(
        // rotate a point (0,1,0) around z
        -sin(v), std::cos(v), 0 );
    // rotate this point around y
    P.z = -P.x * std::sin(u);
    P.x =  P.x * std::cos(u);
    return P;
}


template <typename F>
inline MO_GLM_DETAIL::tvec3<F> normalize_safe(
        const MO_GLM_DETAIL::tvec3<F>& v)
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
