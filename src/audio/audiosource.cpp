/** @file audiosource.cpp

    @brief Audio source belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 5/2/2014</p>
    <p>really started 7/23/2014</p>
*/

#include "audiosource.h"

namespace MO {
namespace AUDIO {


AudioSource::AudioSource(const QString& id, Object *parent)
    : object_ (parent),
      idName_ (id)
{
}


void AudioSource::setNumberThreads(uint num)
{
    transformation_.resize(num);
    sample_.resize(num);
    bufferSize_.resize(num);
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

} // namespace AUDIO
} // namespace MO
