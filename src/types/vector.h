/** @file

    @brief basic definition of vector/matrix types (from glm library)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_TYPES_VECTOR_H
#define MOSRC_TYPES_VECTOR_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

//#include <glm/gtx/simd_vec4.hpp>
//#include <glm/gtx/simd_mat4.hpp>

#include "types/float.h"

namespace MO {

/** @todo fix wrapper for newest glm (9.7+) or replace glm at all!! */
#if GLM_VERSION >= 95

    // -------- basic vector and matrix types -------

    typedef glm::detail::tvec2<Float, glm::defaultp> Vec2;
    typedef glm::detail::tvec3<Float, glm::defaultp> Vec3;
    typedef glm::detail::tvec4<Float, glm::defaultp> Vec4;

    typedef glm::detail::tmat3x3<Float, glm::defaultp> Mat3;
    typedef glm::detail::tmat4x4<Float, glm::defaultp> Mat4;
    //typedef glm::simdMat4 Mat4;

    typedef glm::detail::tvec2<Double, glm::defaultp> DVec2;
    typedef glm::detail::tvec3<Double, glm::defaultp> DVec3;
    typedef glm::detail::tvec4<Double, glm::defaultp> DVec4;

    typedef glm::detail::tmat3x3<Double, glm::defaultp> DMat3;
    typedef glm::detail::tmat4x4<Double, glm::defaultp> DMat4;



// ------------ iostream bindings ---------------

/** stream output of a vector */
template <typename F, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec2<F, P>& v)
{
    s << '<' << v[0] << ", " << v[1] << '>';
    return s;
}

/** stream output of a vector */
template <typename F, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec3<F, P>& v)
{
    s << '<' << v[0] << ", " << v[1] << ", " << v[2] << '>';
    return s;
}

/** stream output of a vector */
template <typename F, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tvec4<F, P>& v)
{
    s << '<' << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << '>';
    return s;
}

/** stream output of a 3by3 matrix */
template <typename F, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat3x3<F, P>& m)
{
    for (int i=0; i<3; ++i)
    {
        s << "<" << m[0][i] << ", " << m[1][i] << ", " << m[2][i] << ">" << std::endl;
    }
    return s;
}

/** stream output of a 4by4 matrix */
template <typename F, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::detail::tmat4x4<F, P>& m)
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


#else // GLM_VERSION


// -------- basic vector and matrix types -------

typedef glm::detail::tvec2<Float> Vec2;
typedef glm::detail::tvec3<Float> Vec3;
typedef glm::detail::tvec4<Float> Vec4;

typedef glm::detail::tmat3x3<Float> Mat3;
typedef glm::detail::tmat4x4<Float> Mat4;
//typedef glm::simdMat4 Mat4;

typedef glm::detail::tvec2<Double> DVec2;
typedef glm::detail::tvec3<Double> DVec3;
typedef glm::detail::tvec4<Double> DVec4;

typedef glm::detail::tmat3x3<Double> DMat3;
typedef glm::detail::tmat4x4<Double> DMat4;



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



#endif



} // namespace MO


#endif // MOSRC_TYPES_VECTOR_H
