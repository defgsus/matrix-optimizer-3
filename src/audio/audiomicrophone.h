/** @file audiomicrophone.h

    @brief A microphone belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#ifndef MOSRC_AUDIO_AUDIOMICROPHONE_H
#define MOSRC_AUDIO_AUDIOMICROPHONE_H

#include <vector>

#include <QString>

#include "types/int.h"
#include "types/float.h"
#include "types/vector.h"
#include "object/object_fwd.h"

namespace MO {
namespace AUDIO {


class AudioMicrophone
{
public:
    /** Constructs the microphone for a particular object.
        The @p id is not really important and only used for displaying. */
    AudioMicrophone(const QString& id, Object * parent);

    // ---------- getter -------------

    /** Returns the id/name of the microphone. */
    const QString& idName() const { return idName_; }

    /** Returns the object, this microphone belongs to */
    Object * object() const { return object_; }

    /** Returns the number of threads runnable on this microphone */
    uint numberThreads() const { return numberThreads_; }

    /** Returns the audio block size for the given thread. */
    uint bufferSize(uint thread) const { return bufferSize_[thread]; }

    /** Returns the set transformation for the given thread */
    const Mat4& transformation(uint thread, uint sample) const { return transformation_[thread][sample]; }

    uint sampleRate() const { return sampleRate_; }

    // ---------- setter -------------

    void setNumberThreads(uint num);
    void setBufferSize(uint samples, uint thread);

    void setTransformation(const Mat4& t, uint thread, uint sample);
    void setTransformation(const Mat4* transformationBlock, uint thread);

    void setSampleRate(uint sr);

    // ----------- audio -------------

    void clearOutputBuffer(uint thread);

    /** Spatially sample the audio source and add the result to buffer */
    void sampleAudioSource(const AUDIO::AudioSource *src, uint thread);

    const F32 * outputBuffer(uint thread) const { return &outputBuffer_[thread][0]; }

private:

    Object * object_;
    QString idName_;

    uint sampleRate_;
    F32 sampleRateInv_;

    uint numberThreads_;
    std::vector<uint> bufferSize_;

    /** [thread][bufferSize] */
    std::vector<std::vector<F32>> outputBuffer_;

    /** [thread][bufferSize] */
    std::vector<std::vector<Mat4>> transformation_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_AUDIOMICROPHONE_H
