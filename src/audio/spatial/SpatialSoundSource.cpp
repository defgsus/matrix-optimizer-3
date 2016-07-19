/** @file spatialsoundsource.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#include "SpatialSoundSource.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/Delay.h"

namespace MO {
namespace AUDIO {

SpatialSoundSource::SpatialSoundSource(AudioBuffer * b, AudioDelay * d)
    : p_signal_             (b)
    , p_signalDist_         (nullptr)
    , p_delay_              (d)
    , p_delayDist_          (nullptr)
    , p_transform_          (b->blockSize())
    , p_isDistSound_        (false)
{
}

uint SpatialSoundSource::bufferSize() const
{
    return p_signal_->blockSize();
}

void SpatialSoundSource::setDistanceSound(bool enable)
{
    if (p_isDistSound_ == enable)
        return;
    p_isDistSound_ = enable;
    if (!p_isDistSound_)
    {
        delete p_signalDist_; p_signalDist_ = nullptr;
        delete p_delayDist_; p_delayDist_ = nullptr;
    }
    else
    {
        p_signalDist_ = new AudioBuffer(p_signal_->blockSize(), p_signal_->numBlocks());
        p_delayDist_ = new AudioDelay(p_delay_->size());
    }
}

} // namespace AUDIO
} // namespace MO
