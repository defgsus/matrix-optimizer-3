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
        This will have the same buffer size as the AudioBuffer given to the constructor. */
    TransformationBuffer * transformationBuffer() { return &p_transform_; }
    const TransformationBuffer * transformationBuffer() const { return &p_transform_; }

private:

    AudioBuffer * p_signal_;
    AudioDelay * p_delay_;
    TransformationBuffer p_transform_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_SPATIAL_SPATIALSOUNDSOURCE_H
