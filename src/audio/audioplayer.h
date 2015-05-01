/** @file audioplayer.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.04.2015</p>
*/

#ifndef MOSRC_AUDIO_AUDIOPLAYER_H
#define MOSRC_AUDIO_AUDIOPLAYER_H

#include "types/float.h"

namespace MO {
namespace AUDIO {


class SoundFile;
class AudioPlayerPrivate;
class AudioPlayerData;





/** Static multi-channel/purpose audio player.

    Currently this conflicts with the actual LiveAudioEngine.

    AND... Currently this is not finished nor used
*/
class AudioPlayer
{
public:

    /** Valid after open(), 44100 otherwise */
    static size_t sampleRate();
    /** Valid after open(), 2 otherwise */
    static size_t numChannels();

    static bool open();
    static bool close();

    static bool stop();

    static bool play(AudioPlayerData *);
    //static bool play(SoundFile * );

private:

    AudioPlayer() { }
    ~AudioPlayer() { }

    static AudioPlayerPrivate * p_();
};


} // namespace AUDIO
} // namespace MO


#endif // MOSRC_AUDIO_AUDIOPLAYER_H
