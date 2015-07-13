/** @file fftwavetablegenerator.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.10.2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_FFTWAVETABLEGENERATOR_H
#define MOSRC_AUDIO_TOOL_FFTWAVETABLEGENERATOR_H

#include "wavetable.h"
#include "math/timeline1d.h"
#include "math/fft.h"
#include "math/constants.h"
#include "types/int.h"

namespace MO {
namespace AUDIO {

template <typename F>
class FftWavetableGenerator
{
public:
    FftWavetableGenerator(uint size = 4096)
        : size_(size)
    { }

    // ----------- getter --------------

    /** Fills the given Wavetable */
    void getWavetable(Wavetable<F> *) const;

    /** Returns a new Wavetable class from the current settings */
    Wavetable<F> * getWavetable() const;

    /** Table and fft size */
    uint size() const { return size_; }

    /** Frequency curve */
    const MATH::Timeline1d & frequencies() const { return freqs_; }

    // ---------- setter ---------------

    /** Sets the size of the wavetable, which will be matched to the next power of two. */
    void setSize(uint size) { size_ = nextPowerOfTwo(size); }

    /** Sets the frequency curve */
    void setFrequencies(const MATH::Timeline1d & tl) { freqs_ = tl; }

    /** Sets the phase curve */
    void setPhases(const MATH::Timeline1d & tl) { phases_ = tl; }

private:

    uint size_;

    MATH::Timeline1d freqs_, phases_;
};

// -------------- template implementation -------------------

template <typename F>
Wavetable<F> * FftWavetableGenerator<F>::getWavetable() const
{
    auto w = new Wavetable<F>;
    getWavetable(w);
    return w;
}

template <typename F>
void FftWavetableGenerator<F>::getWavetable(Wavetable<F> * w) const
{
    w->setSize(size_);
    w->clear();

#ifndef THROUGH_FFT_HERE

    // prepare fft
    MATH::Fft<F> fft;
    fft.setSize(size_);

    // copy/transform frequency data
    const uint n = size_/2;
    for (uint i=0; i<n; ++i)
    {
        const F t = F(i) / (n-1),
                // scale timeline lookup with tablesize
                // which makes a fixed number of bands for all sizes
                bandx = t * n,
                // read timeline curves
                amp = freqs_.get(bandx),
                ph = MATH::moduloSigned(phases_.get(bandx), 1.0);

                //si = std::sin(ph * TWO_PI),
                //co = std::cos(ph * TWO_PI);

        // XXX phase hacked to make it faster (can't afford sin/cos here)
        fft.buffer()[i+1] = F(0.5) * amp * ph;
        fft.buffer()[size_-1-i] = F(-0.5) * amp * (F(1) - ph);
    }

    // create audio data
    fft.ifft();

    // copy into wavetable
    w->setData(fft.buffer());

#else
    for (uint i=0; i<w->size(); ++i)
    {
        const F x = F(i) / w->size();

        for (uint j=1; j<w->size(); ++j)
        {
            const F y = F(j) / w->size();
            const F ph = (x * j + phases_.get(y)) * F(TWO_PI);
            const F amp = freqs_.get(y);

            w->data()[i] += std::sin(ph) * amp;
        }
    }

#endif

    w->normalize();
}



} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_FFTWAVETABLEGENERATOR_H
