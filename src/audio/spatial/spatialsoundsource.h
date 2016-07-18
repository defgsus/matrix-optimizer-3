/** @file spatialsoundsource.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#ifndef MOSRC_AUDIO_SPATIAL_SPATIALSOUNDSOURCE_H
#define MOSRC_AUDIO_SPATIAL_SPATIALSOUNDSOURCE_H

#include "types/vector.h"
#include "math/transformationbuffer.h"

namespace MO {
namespace AUDIO {

class AudioBuffer;
template <typename F, typename I> class Delay;
typedef Delay<F32, int> AudioDelay;

/** Unified type for spatial audio processing.
    Basically a wrapper around some audio buffers.
    Objects can fill these in Object::calculateSoundSourceBuffer()
    */
class SpatialSoundSource
{
public:
    /** Constructs a sound source around an audio buffer */
    SpatialSoundSource(AudioBuffer * buffer, AudioDelay * delay);

    // ------------------ getter ---------------------

    uint bufferSize() const;

    AudioBuffer * signal() { return p_signal_; }
    const AudioBuffer * signal() const { return p_signal_; }

    AudioDelay * delay() { return p_delay_; }
    const AudioDelay * delay() const { return p_delay_; }

    /** Access to the current block of transformations.
        This will have the same buffer size as the AudioBuffer
        given to the constructor. */
    TransformationBuffer * transformationBuffer() { return &p_transform_; }
    const TransformationBuffer * transformationBuffer() const { return &p_transform_; }

    // -------- distance sounds --------

    bool isDistanceSound() const { return p_isDistSound_; }

    /** Enables or disables different audio signals for distant sounds.
        This will create or destroy the signalDist() and delayDist()
        buffers appropriately. */
    void setDistanceSound(bool enable);

    AudioBuffer * signalDist() { return p_signalDist_; }
    const AudioBuffer * signalDist() const { return p_signalDist_; }

    AudioDelay * delayDist() { return p_delayDist_; }
    const AudioDelay * delayDist() const { return p_delayDist_; }


private:

    AudioBuffer * p_signal_, * p_signalDist_;
    AudioDelay * p_delay_, * p_delayDist_;
    TransformationBuffer p_transform_;
    bool p_isDistSound_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_SPATIAL_SPATIALSOUNDSOURCE_H
