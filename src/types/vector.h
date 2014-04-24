/** @file

    @brief basic definition of vector/matrix types (from glm library)

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_TYPES_VECTOR_H
#define MOSRC_TYPES_VECTOR_H

#include <iostream>
#include <glm/glm.hpp>

namespace MO {

    // ------------ basic numeric types -------------

    typedef float Float;

    // -------- basic vector and matrix types -------

    typedef glm::detail::tvec2<Float> Vec2;
    typedef glm::detail::tvec3<Float> Vec3;
    typedef glm::detail::tvec4<Float> Vec4;

    typedef glm::detail::tmat3x3<Float> Mat3;
    typedef glm::detail::tmat4x4<Float> Mat4;

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
