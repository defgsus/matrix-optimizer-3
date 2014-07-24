/** @file wavetable.h

    @brief Wavetable template

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#ifndef MOSRC_AUDIO_WAVETABLE_H
#define MOSRC_AUDIO_WAVETABLE_H

#include <vector>

#include "types/int.h"
#include "math/functions.h"
#include "math/interpol.h"

namespace MO {
namespace AUDIO {



template <typename F>
class Wavetable
{
public:
    Wavetable() { setSize(2); }

    // ----------- getter --------------

    /** Returns the size of the wavetable, which is always a power of two. */
    uint size() const { return data_.size(); }

    /** Returns a pointer to size() consecutive numbers of type F */
    const F * data() const { return &data_[0]; }

    /** Returns the value of the wavetable for @p t in the range of [0,1].
        @p t can be any positive or negative number and will be wrapped into the range. */
    F value(F t) const;

    // ---------- setter ---------------

    /** Sets the size of the wavetable.
        The actual size will be the next power of two of the given size.
        It will also be at least 2! */
    void setSize(uint size);

    /** Sets the data of the wavetable.
        @p ptr is expected to point at size() consecutive entries of type F */
    void setData(const F * ptr);

private:
    std::vector<F> data_;
    uint mask_;
};


template <typename F>
void Wavetable<F>::setSize(uint size)
{
    size = nextPowerOfTwo(std::max((uint)2, size));
    data_.resize(size);
    mask_ = size - 1;
}

template <typename F>
F Wavetable<F>::value(F t)
{
    t = MATH::moduloSigned(t, F(1)) * size();
    const uint pos = t;
    const F frac = F(1) - (t - pos);
    const uint dpos = pos + size_;

    return MATH::interpol_6(frac,
                            data_[(dpos-2) & mask_],
                            data_[(dpos-1) & mask_],
                            data_[(dpos  ) & mask_],
                            data_[(dpos+1) & mask_],
                            data_[(dpos+2) & mask_],
                            data_[(dpos+3) & mask_]
                            );
}


} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_WAVETABLE_H
