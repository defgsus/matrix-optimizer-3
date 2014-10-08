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
class MidiDevice;
class MidiDevices;
class MidiEvent;

class BandlimitWavetableGenerator;
class ButterworthFilter;
class ChebychevFilter;
class EnvelopeFollower;
template <typename F> class EnvelopeGenerator;
class Filter24;
class FixedFilter;
template <typename F> class FloatGate;
class MultiFilter;
template <typename F> class NoteFreq;
class SoundFile;
class SoundFileManager;
class Synth;
class Waveform;
template <typename F> class Wavetable;
class WavetableGenerator;

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_FWD_H
