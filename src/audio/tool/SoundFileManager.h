/** @file soundfilemanager.h

    @brief Singleton manager for all sound files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H
#define MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H

#include <QString>

namespace MO {
namespace AUDIO {

class SoundFile;
class Configuration;

/** Singleton class to manager SoundFile instances */
class SoundFileManager
{
    SoundFileManager();
    ~SoundFileManager();

public:

    /** Returns the soundfile for the given filename.
        When finished with it, call releaseSoundFile()!
        This function always returns a SoundFile class.
        If loading has failed, SoundFile::isOk() will be false.
        The error string is *currently* just printed to the console. */
    static SoundFile * getSoundFile(const QString& filename, bool loadToMemory = true);

    /** Creates a new instance for memory-based recording */
    static SoundFile * createSoundFile(uint channels, uint samplerate);

    /** Tell the manager, you don't need the SoundFile anymore.
        Once all clients have released it, it will be deleted. */
    static void releaseSoundFile(SoundFile*);

    /** Tell the manager that you want a reference on the SoundFile. */
    static void addReference(SoundFile*);

private:

    static SoundFileManager * p_getInstance_();
    static SoundFileManager * p_instance_;

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H
