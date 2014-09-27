/** @file random.h

    @brief random number generators

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/2/2014</p>
*/

#ifndef MOSRC_MATH_RANDOM_H
#define MOSRC_MATH_RANDOM_H


#include <limits>
#include <cstdlib>

namespace MO {
namespace MATH {


template <typename F>
F rnd(F min, F max)
{
    return min + F(std::rand()) / RAND_MAX * (max - min);
}


/** a random number generator.
    <p>UI must be an <b>unsigned</b> integer type</p>
    <p>F must be a float type</p> */
template <typename UI = unsigned int, typename F = float>
class Random
{
    public:

    /** creator with optional seed */
    Random(UI seed = 1007)
        :	seed_	(seed)
    { }

    /** set a new seed */
    void seed(UI seed) { seed_ = seed; }

    /** return a random number in the range of type UI */
    UI rand()
    {
        seed_ = seed_ * 196314165 + 907633515;
        return seed_;
    }

    /** return a random number in the range [min_val, max_val] */
    F rand(F min_val, F max_val)
    {
        F range = (max_val - min_val);
        return range?
            min_val + (F)rand() / max() * range
            : min_val;
    }

    /** return the maximum value that rand() can return */
    UI max() const { return std::numeric_limits<UI>::max(); }

    private:

    UI seed_;
};

} // namespace MATH
} // namespace MO


#endif // MOSRC_MATH_RANDOM_H
