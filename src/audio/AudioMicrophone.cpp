/** @file audiomicrophone.cpp

    @brief A microphone belonging to a MO::Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#if 0

#include "audiomicrophone.h"
#include "io/log.h"
#include "io/error.h"
#include "math/interpol.h"
#include "audiosource.h"

namespace MO {
namespace AUDIO {


AudioMicrophone::AudioMicrophone(const QString& id, Object *parent)
    : object_       (parent),
      idName_       (id),
      sampleRate_   (1),
      sampleRateInv_(1),
      numberThreads_(0)
{
}


void AudioMicrophone::setNumberThreads(uint num)
{
    bufferSize_.resize(num);
    outputBuffer_.resize(num);
    transformation_.resize(num);
}

void AudioMicrophone::setBufferSize(uint samples, uint thread)
{
    bufferSize_[thread] = samples;

    transformation_[thread].resize(samples);

    for (auto &t : transformation_[thread])
        t = Mat4(1);

    outputBuffer_[thread].resize(samples);
    clearOutputBuffer(thread);
}

void AudioMicrophone::setSampleRate(uint sr)
{
    MO_ASSERT(sr>0, "bogus samplerate");

    sampleRate_ = std::max((uint)1, sr);
    sampleRateInv_ = F32(1) / sampleRate_;
}

void AudioMicrophone::setTransformation(const Mat4& t, uint thread, uint sample)
{
    MO_ASSERT(thread < transformation_.size(), "thread " << thread << " out of range");
    MO_ASSERT(sample < transformation_[thread].size(), "sample " << sample << " out of range");

    memcpy(&transformation_[thread][sample], &t, sizeof(Mat4));
}

void AudioMicrophone::setTransformation(const Mat4 *block, uint thread)
{
    const uint size = bufferSize_[thread];
#if (1)
    /*for (uint i = 0; i<size; ++i)
        transformation_[thread][i] = *block++;
    */
    memcpy(&transformation_[thread][0], block, size * sizeof(Mat4));
#else
    for (uint i=0; i<size; ++i)
    {
        const Float t = Float(i) / size;
        transformation_[thread][0] =
                block[0] + t * (block[size-1] - block[0]);
    }
#endif
}


void AudioMicrophone::clearOutputBuffer(uint thread)
{
    for (auto &s : outputBuffer_[thread])
        s = 0.f;
}

void AudioMicrophone::sampleAudioSource(
        const AUDIO::AudioSource *src, uint thread)
{
    const uint size = bufferSize(thread);

    MO_ASSERT(size == src->bufferSize(thread), "unmatched buffer size");

    F32 * buffer = &outputBuffer_[thread][0];

    for (uint i=0; i<size; ++i)
    {
        const Mat4&
                mmic = transformation(thread, i),
                msnd = src->transformation(thread, i);
        const Vec4
                posMic = mmic * Vec4(0,0,0,1),
                posSnd = msnd * Vec4(0,0,0,1);

        // direction towards sound
        /*F32 dx = msnd[3][0] - mmic[3][0],
            dy = msnd[3][1] - mmic[3][1],
            dz = msnd[3][2] - mmic[3][2];
            */
        F32 dx = posSnd.x - posMic.x,
            dy = posSnd.y - posMic.y,
            dz = posSnd.z - posMic.z;

        // distance to sound
        const F32 dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (dist == 0.f)
            *buffer++ += src->getSample(thread, i);
        else
        {
            // amplitude from distance
            const F32 ampDist = 1.f / (1.f + dist);

            // delaytime from distance
            const F32 delaySam = dist / 330.f * sampleRate();

            // delayed sample
#if (1)
            const F32 sam = src->getDelaySample(thread, i, delaySam);
#else
            const F32 sam = src->getSample(thread, i);
#endif
            // normalize direction
            dx /= dist;
            dy /= dist;
            dz /= dist;

            // get direction of microphone
            // (suppose microphone originally points at <0,0,-1>)
#if (0)
            F32
                mx =  mmic[0][2],
                my =  mmic[1][2],
                mz = -mmic[2][2];
#else
            Vec4 mdir_ = mmic * Vec4(0,0,-1, 0);
            F32 mx = mdir_[0], my = mdir_[1], mz = mdir_[2];
#endif
            // normalize
            const F32 mdirmag = std::sqrt(mx*mx + my*my + mz*mz);
            mx /= mdirmag;
            my /= mdirmag;
            mz /= mdirmag;

            // direction factor
            const F32 mdot = 0.5f + 0.5f * (dx * mx + dy * my + dz * mz);

            // amplitude from direction
            const F32 ampDir = std::pow(mdot, 3.f);

            *buffer++ += sam * ampDist * ampDir;
        }
    }
}



} // namespace AUDIO
} // namespace MO

#endif
