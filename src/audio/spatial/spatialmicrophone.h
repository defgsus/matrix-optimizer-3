/** @file spatialmicrophone.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#ifndef MOSRC_AUDIO_SPATIAL_SPATIALMICROPHONE_H
#define MOSRC_AUDIO_SPATIAL_SPATIALMICROPHONE_H

#include "types/vector.h"
#include "math/transformationbuffer.h"

namespace MO {
namespace AUDIO {

class AudioBuffer;
class SpatialSoundSource;

class SpatialMicrophone
{
public:
    /** Constructs a microphone around an audio buffer */
    SpatialMicrophone(AudioBuffer * buffer, uint sampleRate);

    // ------------------ getter ---------------------

    uint bufferSize() const;

    /** The recorded signal */
    AudioBuffer * signal() { return p_signal_; }
    const AudioBuffer * signal() const { return p_signal_; }

    /** Access to the current block of transformations.
        This will have the same buffer size as the AudioBuffer given to the constructor. */
    TransformationBuffer * transformationBuffer() { return &p_transform_; }
    const TransformationBuffer * transformationBuffer() const { return &p_transform_; }

    // -------------- spatialization -----------------

    void spatialize(const QList<SpatialSoundSource*>& soundSources);

private:

    void spatialize_(SpatialSoundSource*);

    AudioBuffer * p_signal_;
    TransformationBuffer p_transform_;
    uint p_sampleRate_;
    F32 p_sampleRateInv_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_SPATIAL_SPATIALMICROPHONE_H
