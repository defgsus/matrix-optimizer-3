/** @file hash.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_MATH_HASH_H
#define MOSRC_MATH_HASH_H

namespace MO {
namespace MATH {

template <typename I>
struct hash_traits
{
    const static I prime1 = 3685403;
    const static I prime2 = 4898213;
    const static I prime3 = 5037467;
    const static I prime4 = 5688511;
};

template <>
struct hash_traits<long int>
{
    const static long int prime1 = 13512361;
    const static long int prime2 = 13766117;
    const static long int prime3 = 14186981;
    const static long int prime4 = 14764097;
};

template <>
struct hash_traits<long unsigned int>
{
    const static long unsigned int prime1 = hash_traits<long int>::prime1;
    const static long unsigned int prime2 = hash_traits<long int>::prime2;
    const static long unsigned int prime3 = hash_traits<long int>::prime3;
    const static long unsigned int prime4 = hash_traits<long int>::prime4;
};



/** Returns a hash value of type I for the two components of type I1 */
template <typename I, typename I1>
I getHash(I1 x, I1 y)
{
    return    (I)(x * hash_traits<I>::prime1)
            ^ (I)(y * hash_traits<I>::prime2);
}

/** Returns a hash value of type I for the three components of type I1 */
template <typename I, typename I1>
I getHash(I1 x, I1 y, I1 z)
{
    return    (I)(x * hash_traits<I>::prime1)
            ^ (I)(y * hash_traits<I>::prime2)
            ^ (I)(z * hash_traits<I>::prime3);
}

/** Returns a hash value of type I for the four components of type I1 */
template <typename I, typename I1>
I getHash(I1 x, I1 y, I1 z, I1 w)
{
    return    (I)(x * hash_traits<I>::prime1)
            ^ (I)(y * hash_traits<I>::prime2)
            ^ (I)(z * hash_traits<I>::prime3)
            ^ (I)(w * hash_traits<I>::prime4);
}



/** Returns a hash value of type I for the two components of type I1.
    Order of components does not matter. */
template <typename I, typename I1>
I getHashUnordered(I1 x, I1 y)
{
    return    (I)(x * hash_traits<I>::prime1)
            ^ (I)(y * hash_traits<I>::prime1);
}


/** Returns a hash value of type I for the two components */
template <typename I>
I getHash(I x, I y)
{
    return    (x * hash_traits<I>::prime1)
            ^ (y * hash_traits<I>::prime2);
}

/** Returns a hash value of type I for the three components */
template <typename I>
I getHash(I x, I y, I z)
{
    return    (x * hash_traits<I>::prime1)
            ^ (y * hash_traits<I>::prime2)
            ^ (z * hash_traits<I>::prime3);
}

/** Returns a hash value of type I for the four components */
template <typename I>
I getHash(I x, I y, I z, I w)
{
    return    (x * hash_traits<I>::prime1)
            ^ (y * hash_traits<I>::prime2)
            ^ (z * hash_traits<I>::prime3)
            ^ (w * hash_traits<I>::prime4);
}

} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_HASH_H
