/** @file audiosource.cpp

    @brief Audio source belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/2/2014</p>
    <p>really started 7/23/2014</p>
*/

#include "audiosource.h"
#include "io/log.h"
#include "io/error.h"
#include "math/interpol.h"

namespace MO {
namespace AUDIO {


AudioSource::AudioSource(const QString& id, Object *parent)
    : object_ (parent),
      idName_ (id)
{
}


void AudioSource::setNumberThreads(uint num)
{
    bufferSize_.resize(num);
    transformation_.resize(num);
    sample_.resize(num);
    history_.resize(num);
    historyPos_.resize(num);
}

void AudioSource::setBufferSize(uint samples, uint thread)
{
    bufferSize_[thread] = samples;

    transformation_[thread].resize(samples);
    sample_[thread].resize(samples);

    for (auto &t : transformation_[thread])
        t = Mat4(1.0);

    for (auto &s : sample_[thread])
        s = 0.f;

}

void AudioSource::setDelaySize(uint samples, uint thread)
{
    history_[thread].resize(samples);
    historyPos_[thread] = 0;

    for (auto &s : history_[thread])
        s = 0.f;
}

void AudioSource::setTransformation(const Mat4 *block, uint thread)
{
    const uint size = bufferSize_[thread];
    for (uint i = 0; i<size; ++i)
        transformation_[thread][i] = *block++;
}

void AudioSource::setSample(F32 *audioBlock, uint thread)
{
    const uint size = bufferSize_[thread];
    for (uint i = 0; i<size; ++i)
        sample_[thread][i] = *audioBlock++;
}

void AudioSource::pushDelay(uint thread)
{
    if (history_[thread].empty())
        return;

    const uint
        size = bufferSize_[thread],
        mask = history_[thread].size() - 1;

    const F32
            *src = &sample_[thread][0],
            *end = src + size;

    historyPos_[thread] = (historyPos_[thread] + size) & mask;

    for (uint j = historyPos_[thread]; src != end; ++j)
    {
        history_[thread][j & mask] = *src++;
    }
}

F32 AudioSource::getDelaySample(uint thread, uint sample, F32 delayPos) const
{
    if (history_[thread].empty())
        return getSample(thread, sample);

    const uint
        len = history_[thread].size(),
        mask = len - 1,
        hpos = historyPos_[thread] + sample + len;

    const F32
        *src = &history_[thread][0];

    const uint pos = delayPos;
    const F32 fade = 1.f - (delayPos - pos);

    const uint dpos = hpos - pos;

    // no interpolation
    //return src[(d) & mask];

    return MATH::interpol_6(fade,
                            src[(dpos - 2) & mask],
                            src[(dpos - 1) & mask],
                            src[(dpos) & mask],
                            src[(dpos + 1) & mask],
                            src[(dpos + 2) & mask],
                            src[(dpos + 3) & mask]
                            );
/*
    MO_ASSERT(history_[thread].size() > readpos, "delaytime exceeded, "
              "is " << history_[thread].size() << ", requested "
              << readpos);
*/
}


} // namespace AUDIO
} // namespace MO
