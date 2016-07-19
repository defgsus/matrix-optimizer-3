/** @file spatialmicrophone.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.12.2014</p>
*/

#include "SpatialMicrophone.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/Delay.h"
#include "audio/spatial/SpatialSoundSource.h"
#include "io/error.h"

namespace MO {
namespace AUDIO {

SpatialMicrophone::SpatialMicrophone(AudioBuffer * b, uint sampleRate, uint channel)
    : p_signal_             (b),
      p_transform_          (b->blockSize()),
      p_sampleRate_         (sampleRate),
      p_channel_            (channel),
      p_sampleRateInv_      (1.f / std::max(uint(1), sampleRate))
    , p_amp_                (1.f)
    , p_dirExp_             (3.f)
    , p_distFade_           (1.f)
    , p_minDist_            (5.f)
    , p_maxDist_            (50.f)
    , p_enableDist_         (false)
{
}

uint SpatialMicrophone::bufferSize() const
{
    return p_signal_->blockSize();
}


void SpatialMicrophone::spatialize(const QList<SpatialSoundSource*>& sources)
{
    // clear accumulation buffer
    p_signal_->writeNullBlock();

    // sample each sound source
    for (auto s : sources)
        spatialize_(s);
}

void SpatialMicrophone::spatialize_(SpatialSoundSource * snd)
{
    MO_ASSERT(snd->bufferSize() == bufferSize(), "unmatched buffer size"
              << snd->bufferSize() << "/" << bufferSize());

    const F32 EPSILON = 1e-20,
            dist_fac = 1.f / std::max(0.00001f, p_maxDist_ - p_minDist_);

    // add-write here
    F32 * buffer = p_signal_->writePointer();

    F32 delayReadPos = bufferSize();

    for (uint i=0; i<bufferSize(); ++i, --delayReadPos)
    {
        // get transformation and position for mic/snd for each sample
        const Mat4&
                matrixMic = transformationBuffer()->transformation(i),
                matrixSnd = snd->transformationBuffer()->transformation(i);
        const Vec4
                posMic = matrixMic * Vec4(0,0,0,1),
                posSnd = matrixSnd * Vec4(0,0,0,1);

        // direction towards sound
        F32 dx = posSnd.x - posMic.x,
            dy = posSnd.y - posMic.y,
            dz = posSnd.z - posMic.z;

        // distance to sound
        const F32 dist = std::sqrt(dx*dx + dy*dy + dz*dz);

        // mic and sound are at same position?
        if (dist < EPSILON)
            *buffer++ += snd->signal()->read(i);
        else
        {
            // amplitude from distance
            const F32 ampDist = 1.f / (1.f + p_distFade_ * dist);

            // delaytime from distance
            const F32 delaySam = dist / 330.f * p_sampleRate_;

            // read delayed sample from snd
#if (1)
            F32 sam = snd->delay()->read(delayReadPos + delaySam);
#else
            F32 sam = snd->signal()->read(i);
#endif
            // mix with distance-sound
            if (p_enableDist_ && snd->isDistanceSound())
            {
                F32 dmix = std::max(0.f, std::min(1.f,
                                (dist - p_minDist_) * dist_fac ));

                sam += dmix * (snd->delayDist()->read(delayReadPos + delaySam) - sam);
            }


            // normalize direction
            dx /= dist;
            dy /= dist;
            dz /= dist;

            // get direction of microphone
            // (suppose microphone originally points at <0,0,-1>)
            Vec4 micDir = matrixMic * Vec4(0,0,-1, 0);
            F32 mx = micDir[0], my = micDir[1], mz = micDir[2];

            // normalize
            const F32 micDirMag = std::sqrt(mx*mx + my*my + mz*mz);
            mx /= micDirMag;
            my /= micDirMag;
            mz /= micDirMag;

            // direction factor
            const F32 micDot = 0.5f + 0.5f * (dx * mx + dy * my + dz * mz);

            // amplitude from direction
            const F32 ampDir = std::pow(micDot, p_dirExp_);

            // add to microphone sample buffer
            *buffer++ += sam * ampDist * ampDir * p_amp_;
        }
    }
}


} // namespace AUDIO
} // namespace MO
