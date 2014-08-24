/** @file soundfile.cpp

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include "soundfile.h"

namespace MO {
namespace AUDIO {

SoundFile::SoundFile()
    : sr_       (1),
      lenSam_   (0),
      lenSec_   (0)
{
}

Double SoundFile::value(Double time) const
{
    return 0;
}


} // namespace AUDIO
} // namespace MO
