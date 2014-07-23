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
    int threads() const { return transformation_.size(); }

    /** Returns the set transformation for the given thread */
    const Mat4& transformation(int thread) const { return transformation_[thread]; }

    F32 getSample(int thread) const { return sample_[thread]; }

    // ---------- setter -------------

    void setNumberThreads(int num);

    void setTransformation(const Mat4& t, int thread) { transformation_[thread] = t; }

    void setSample(F32 sample, int thread) { sample_[thread] = sample; }

    //void setBufferSize(uint samples) { buffer_.resize(samples); }

private:

    Object * object_;
    QString idName_;

    std::vector<F32> sample_;

    std::vector<Mat4> transformation_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_AUDIOSOURCE_H
