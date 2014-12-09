/** @file spatialsoundsource.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#include "spatialsoundsource.h"
#include "audio/tool/audiobuffer.h"

namespace MO {
namespace AUDIO {

SpatialSoundSource::SpatialSoundSource(AudioBuffer * b)
    : p_signal_             (b),
      p_transform_          (b->blockSize())
{
}

uint SpatialSoundSource::bufferSize() const
{
    return p_signal_->blockSize();
}


} // namespace AUDIO
} // namespace MO
