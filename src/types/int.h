/** @file int.h

    @brief Integer types

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_TYPES_INT_H
#define MOSRC_TYPES_INT_H

namespace MO {

    /** General unsigned int type */
    typedef unsigned int uint;


    namespace Private
    {
        template <typename I>
        constexpr I nextPowerOfTwo_(I num, I bit)
        {
            return num > I(1)<<bit ? nextPowerOfTwo_(num, I(bit + 1)) : I(1<<bit);
        }
    } // namespace Private

    /** Returns the next power of two for the given integer.
        Type overflows are not handled, and generally return zero or negative numbers. */
    template <typename I>
    constexpr I nextPowerOfTwo(I in) { return Private::nextPowerOfTwo_(in, (I)0); }

} // namespace MO

#endif // MOSRC_TYPES_INT_H
