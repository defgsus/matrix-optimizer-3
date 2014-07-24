/** @file wavetablegenerator.h

    @brief Wavetable generator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#ifndef MOSRC_AUDIO_WAVETABLEGENERATOR_H
#define MOSRC_AUDIO_WAVETABLEGENERATOR_H

#include "wavetable.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {

class WavetableGenerator
{
public:
    WavetableGenerator();

    // ----------- getter --------------

    /** Fills the given Wavetable */
    template <typename F>
    void getWavetable(Wavetable<F> *) const;

    /** Returns a new Wavetable class from the current settings */
    template <typename F>
    Wavetable<F> * getWavetable() const;

    // ---------- setter ---------------


private:

    uint size_;
    uint numPartials_;
    uint baseOctave_;
    uint octaveStep_;
    Double amplitudeMult_;
    Double freqFac_;
};

// -------------- template implementation -------------------

template <typename F>
Wavetable<F> * WavetableGenerator::getWavetable() const
{
    auto w = new Wavetable<F>;
    getWavetable(w);
    return w;
}

template <typename F>
void WavetableGenerator::getWavetable(Wavetable<F> * w) const
{
    w->setSize(size_);
    w->clearData();

    uint oct = baseOctave_;
    Double amp = F(1);

    for (uint j=0; j<numPartials_; ++j)
    {
        F fac = freqFac_ * oct;

        for (uint i=0; i<size_; ++i)
        {
            F t = F(i) / size_;
            w->data()[i] += std::sin(t * fac);
        }

        oct *= octaveStep_;
        amp *= amplitudeMult_;
    }
}



} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_WAVETABLEGENERATOR_H
