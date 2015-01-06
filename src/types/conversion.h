/** @file conversion.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#ifndef MOSRC_TYPES_CONVERSION_H
#define MOSRC_TYPES_CONVERSION_H

#include "float.h"
#include "int.h"

namespace MO {

namespace Private {

    template <typename From, typename To>
    struct conversion_traits
    {
        static To convert(From) { return To::types_not_implemented_; }
    };

    template <>
    struct conversion_traits<Double, unsigned int>
    {
        static unsigned int convert(Double x) { return x; /* TODO */ }
    };

    template <>
    struct conversion_traits<Double, long>
    {
        static long convert(Double x) { return lrint(x); }
    };

    template <>
    struct conversion_traits<Double, long long>
    {
        static long long convert(Double x) { return llrint(x); }
    };

    template <>
    struct conversion_traits<float, int>
    {
        static int convert(float x) { return x; /* TODO */ }
    };

    template <>
    struct conversion_traits<float, unsigned int>
    {
        static unsigned int convert(float x) { return x; /* TODO */ }
    };


} // namespace Private


/** Fast conversion of one type to another */
template <typename From, typename To>
inline To convert(From x)
{
    return Private::conversion_traits<From, To>::convert(x);
}


} // namespace MO

#endif // MOSRC_TYPES_CONVERSION_H
