/** @file streamoperators_glbinding.h

    @brief glbinding types to std::basic_ostream

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/20/2014</p>
*/

#ifndef MOSRC_IO_STREAMOPERATORS_GLBINDING_H
#define MOSRC_IO_STREAMOPERATORS_GLBINDING_H

#include <ostream>
#include "gl/opengl.h"

#include <glbinding/Meta.h>

namespace MO {

template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, gl::GLenum e)
{
    o << glbinding::Meta::getString(e);
    return o;
}

template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, gl::GLboolean e)
{
    o << glbinding::Meta::getString(e);
    return o;
}

template <typename T>
std::basic_ostream<T>& operator << (std::basic_ostream<T>& o, gl::GLextension e)
{
    o << glbinding::Meta::getString(e);
    return o;
}

} // namespace MO

#endif // MOSRC_IO_STREAMOPERATORS_GLBINDING_H
