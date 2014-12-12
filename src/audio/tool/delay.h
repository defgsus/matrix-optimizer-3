/** @file delay.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#include <vector>
#include <cassert>

#include "types/int.h" // for nextPowerOfTwo()
#include "types/conversion.h"
#include "math/functions.h"
#ifndef MO_DISABLE_DELAY_INTERPOLATION
#include "math/interpol.h"
#endif

#ifndef MOSRC_AUDIO_TOOL_DELAY_H
#define MOSRC_AUDIO_TOOL_DELAY_H

namespace MO {
namespace AUDIO {


/** Delay class for sequential block write and interpolated sample-wise read.
    Type F must be a float type, used as sample and for sub-sample position.
    Type I must be a signed integer type.
*/
template <typename F, typename I>
class Delay
{
    static_assert(std::is_floating_point<F>::value, "F must be floating point");
    static_assert(std::is_signed<I>::value, "I must be signed");

public:
    /** Creates a delay with a particular history size.
        @p bufferSize will be rounded to the next power-of-two if necessary. */
    Delay(I bufferSize)
        : p_buffer_     (nextPowerOfTwo(bufferSize)),
          p_mask_       (nextPowerOfTwo(bufferSize) - 1),
          p_pos_        (0)
    {
        assert(bufferSize > 2 && "invalid buffersize for Delay class");
    }

    // --------------- getter ---------------------

    /** Returns the size of the delay history */
    size_t size() const { return p_buffer_.size(); }

    /** Returns a sample from the history. */
    F read(F offset) const;

    // ------------- setter -----------------------

    /** Inserts @p num samples from @p data into the delay history */
    void writeBlock(const F * data, I num);

private:

    std::vector<F> p_buffer_;
    I p_mask_, p_pos_;
};


// default type
typedef Delay<F32, int> AudioDelay;


// _____________________________ templ. impl. _______________________________

template <typename F, typename I>
inline void Delay<F, I>::writeBlock(const F * data, I num)
{
    for (I i = 0; i < num; ++i, ++data)
    {
        p_pos_ = (p_pos_ + 1) & p_mask_;
        p_buffer_[p_pos_] = *data;
    }
}

template <typename F, typename I>
inline F Delay<F, I>::read(F foffset) const
{
    const I
        offset = convert<F, I>(foffset),
        dpos = p_pos_ - offset;

#ifdef MO_DISABLE_DELAY_INTERPOLATION
    // no interpolation
    return p_buffer_[dpos & mask];
#else
    const F fade = F(1) - MATH::frac(foffset);

    return MATH::interpol_6(fade,
                            p_buffer_[(dpos - 2) & p_mask_],
                            p_buffer_[(dpos - 1) & p_mask_],
                            p_buffer_[(dpos    ) & p_mask_],
                            p_buffer_[(dpos + 1) & p_mask_],
                            p_buffer_[(dpos + 2) & p_mask_],
                            p_buffer_[(dpos + 3) & p_mask_]
                            );
#endif
}



} // namespace AUDIO
} // namespace DELAY


#endif // MOSRC_AUDIO_TOOL_DELAY_H
