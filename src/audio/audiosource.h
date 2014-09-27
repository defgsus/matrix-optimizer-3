/** @file audiosource.h

    @brief Audio source belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/2/2014</p>
    <p>really started 7/23/2014</p>
*/

#ifndef MOSRC_AUDIO_AUDIOSOURCE_H
#define MOSRC_AUDIO_AUDIOSOURCE_H

#include <vector>

#include <QString>

#include "types/int.h"
#include "types/float.h"
#include "types/vector.h"
#include "object/object_fwd.h"

namespace MO {
namespace AUDIO {


class AudioSource
{
public:
    /** Constructs the audio source for a particular object.
        The @p id is not really important and only used for displaying. */
    AudioSource(const QString& id, Object * parent);

    // ---------- getter -------------

    /** Returns the id/name of the audio source. */
    const QString& idName() const { return idName_; }

    /** Returns the object, this audio source belongs to */
    Object * object() const { return object_; }

    /** Returns the number of threads runnable on this audio source */
    uint numberThreads() const { return numberThreads_; }

    /** Returns the audio block size for the given thread. */
    uint bufferSize(uint thread) const { return bufferSize_[thread]; }

    /** Returns the length of the delay in samples for the given thread */
    uint delaySize(uint thread) const { return history_[thread].size(); }

    /** Returns the set transformation for the given thread */
    const Mat4& transformation(uint thread, uint sample) const { return transformation_[thread][sample]; }

    /** Returns the sample for the given thread at the given sample position in the block */
    F32 getSample(uint thread, uint sample) const { return sample_[thread][sample]; }

    /** Returns a const pointer to bufferSize(thead) number of samples */
    const F32* getSample(uint thread) const { return &sample_[thread][0]; }

    /** Returns a sample from the history */
    F32 getDelaySample(uint thread, uint sample, F32 delayPos) const;

    // ---------- setter -------------

    void setNumberThreads(uint num);
    void setBufferSize(uint samples, uint thread);
    /** Sets the delaysize for the given thread.
        @note Must be a power of two! */
    void setDelaySize(uint samples, uint thread);

    void setTransformation(const Mat4& t, uint thread, uint sample) { transformation_[thread][sample] = t; }
    void setTransformation(const Mat4* transformationBlock, uint thread);

    void setSample(F32 audio, uint thread, uint sample) { sample_[thread][sample] = audio; }
    void setSample(F32 * audioBlock, uint thread);

    /** Returns a pointer to bufferSize(thead) number of samples for read/write. */
    F32* samples(uint thread) { return &sample_[thread][0]; }

    /** Writes the current sampleblock to the delay history */
    void pushDelay(uint thread);

private:

    Object * object_;
    QString idName_;

    uint numberThreads_;
    std::vector<uint> bufferSize_;

    /** [thread][bufferSize] */
    std::vector<std::vector<F32>> sample_;

    /** [thread][delaylength] */
    std::vector<std::vector<F32>> history_;
    std::vector<uint> historyPos_;

    /** [thread][bufferSize] */
    std::vector<std::vector<Mat4>> transformation_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_AUDIOSOURCE_H
