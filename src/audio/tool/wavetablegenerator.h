/** @file wavetablegenerator.h

    @brief Wavetable generator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_WAVETABLEGENERATOR_H
#define MOSRC_AUDIO_TOOL_WAVETABLEGENERATOR_H

#include "wavetable.h"
#include "types/float.h"
#include "types/int.h"

namespace MO {
namespace IO { class DataStream; }
namespace AUDIO {

class WavetableGenerator
{
public:
    WavetableGenerator();

    /** Store settings */
    void serialize(IO::DataStream&) const;
    /** Restore settings */
    void deserialize(IO::DataStream&);

    // ----------- getter --------------

    /** Fills the given Wavetable */
    template <typename F>
    void getWavetable(Wavetable<F> *) const;

    /** Returns a new Wavetable class from the current settings */
    template <typename F>
    Wavetable<F> * getWavetable() const;

    uint size() const { return size_; }
    uint numPartials() const { return numPartials_; }
    uint baseOctave() const { return baseOctave_; }
    uint octaveStep() const { return octaveStep_; }
    Double amplitudeMultiplier() const { return amplitudeMult_; }
    Double basePhase() const { return basePhase_; }
    Double phaseShift() const { return phaseShift_; }

    // ---------- setter ---------------

    /** Sets the size of the wavetable, which will be matched to the next power of two. */
    void setSize(uint size) { size_ = nextPowerOfTwo(size); }

    /** Sets the number of voices */
    void setNumPartials(uint num) { numPartials_ = num; }

    /** Sets the base octave */
    void setBaseOctave(uint oct);

    /** Sets the number of octaves that will be added for each partial voice */
    void setOctaveStep(uint step) { octaveStep_ = step; }

    /** Sets the multiplier for the amplitude for each partial voice */
    void setAmplitudeMultiplier(Double mult) { amplitudeMult_ = mult; }

    /** Sets the base phase [0,1] */
    void setBasePhase(Double phase);

    /** Sets the value to be added to the phase for each partial note [0,1] */
    void setPhaseShift(Double shift);

private:

    uint
        size_,
        numPartials_,
        baseOctave_,
        octaveStep_;
    Double
        basePhase_,
        phaseShift_,
        amplitudeMult_,
        freqFac_,
        phaseFac_,
        phaseShiftFac_;
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
    w->clear();

    uint oct = baseOctave_;
    Double amp = F(1), amp2 = F(1),
           phase = phaseFac_;

    for (uint j=0; j<numPartials_; ++j)
    {
        F fac = freqFac_ * oct;

        for (uint i=0; i<size_; ++i)
        {
            F t = F(i) / size_;
            w->data()[i] += amp2 * std::sin(t * fac + phase);
        }

        oct += octaveStep_;
        amp *= amplitudeMult_;
        amp2 = amp / oct;
        phase += phaseShiftFac_;
    }

    w->normalize();
}



} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_TOOL_WAVETABLEGENERATOR_H
