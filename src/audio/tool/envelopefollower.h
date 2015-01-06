/** @file envelopefollower.h

    @brief Envelope follower for audio signals

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_ENVELOPEFOLLOWER_H
#define MOSRC_AUDIO_TOOL_ENVELOPEFOLLOWER_H


#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace AUDIO {


class EnvelopeFollower
{
public:
    EnvelopeFollower();

    // ----------- setter ----------------

    /** Sets the samplerate in hertz.
        Requires updateCoefficients() to be called. */
    void setSampleRate(uint sr) { sr_ = sr; }

    /** Sets the speed to follow a rising amplitude in seconds.
        Requires updateCoefficients() to be called. */
    void setFadeIn(F32 speed) { fadeIn_ = speed; }

    /** Sets the speed to follow a decaying amplitude in seconds.
        Requires updateCoefficients() to be called. */
    void setFadeOut(F32 speed) { fadeOut_ = speed; }

    /** Sets the speed to follow the signal average.
        Requires updateCoefficients() to be called. */
    void setAverageSpeed(F32 speed) { fadeAv_ = speed; }

    /** Sets the step value of how much the input signal
        must raise above the current average, default == 0.
        No updateCoefficients() required. */
    void setThreshold(F32 t) { threshold_ = t; }

    /** Sets the amplitude of the input signal before enveloping.
        No updateCoefficients() required. */
    void setInputAmplitude(F32 amp) { ampIn_ = amp; }

    /** Sets the amplitude of the input signal before enveloping.
        No updateCoefficients() required. */
    void setOutputAmplitude(F32 amp) { ampOut_ = amp; }

    // ---------- getter ------------------

    uint sampleRate() const { return sr_; }

    F32 fadeIn() const { return fadeIn_; }
    F32 fadeOut() const { return fadeOut_; }
    F32 averageSpeed() const { return fadeAv_; }
    F32 threshold() const { return threshold_; }
    F32 inputAmplitude() const { return ampIn_; }
    F32 outputAmplitude() const { return ampOut_; }

    /** Returns current envelope (after last processing step) */
    F32 envelope() const { return env_ * ampOut_; }
    /** Returns the current signal average (after last processing step) */
    F32 average() const { return av_ * ampOut_; }

    // ---------- processing --------------

    /** Resets the current envelope.
        E.g. in case a NAN has crawled in. */
    void reset();

    /** Call this after changes to the settings */
    void updateCoefficients();

    /** Consequently call this to follow the input signal.
        Returns the current envelope. */
    F32 process(const F32 * input, uint blockSize)
        { return process(input, 1, blockSize); }

    /** Consequently call this to follow the input signal.
        Returns the current envelope.
        The strides define the spacing between consecutive samples. */
    F32 process(const F32 * input, uint inputStride,
                       uint blockSize);

    /** Also puts the envelope into @p output.
        Consequently call this to follow the input signal.
        Returns the current envelope.
        The strides define the spacing between consecutive samples. */
    F32 process(const F32 * input, F32 * output, uint blockSize)
        { return process(input, 1, output, 1, blockSize); }

    /** Also puts the envelope into @p output.
        Consequently call this to follow the input signal.
        Returns the current envelope.
        The strides define the spacing between consecutive samples. */
    F32 process(const F32 * input, uint inputStride,
                F32 * output, uint outputStride,
                       uint blockSize);

private:

    uint sr_;
    F32 fadeIn_, fadeOut_, fadeAv_,
        threshold_,
        ampIn_, ampOut_,
        qIn_, qOut_, qAv_,
        env_, av_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_ENVELOPEFOLLOWER_H
