/** @file audio_fwd.h

    @brief Forward declarations of audio classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_AUDIO_FWD_H
#define MOSRC_AUDIO_FWD_H

namespace MO {
namespace AUDIO {

class AudioDevice;
class AudioDecices;
class AudioSource;
class Configuration;

class ChebychevFilter;
class EnvelopeFollower;
class MultiFilter;
class SoundFile;
class SoundFileManager;
class Waveform;
template <typename F> class Wavetable;
class WavetableGenerator;

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_FWD_H
