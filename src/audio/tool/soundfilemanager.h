/** @file soundfilemanager.h

    @brief Singleton manager for all sound files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H
#define MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H

namespace MO {
namespace AUDIO {

class SoundFile;

class SoundFileManager
{
    SoundFileManager();
    ~SoundFileManager();

public:

    /** Returns the soundfile for the given filename.
        When you are finished with it, call releaseSoundFile()!
        This function always returns a SoundFile class.
        If loading has failed, SoundFile::ok() will be false.
        The error string is *currently* just printed to the console. */
    static SoundFile * getSoundFile(const QString& filename);

    /** Tell the manager, you don't need the SoundFile anymore.
        Once all clients have released it, it will be deleted. */
    static void releaseSoundFile(SoundFile*);

private:

    static SoundFileManager * getInstance_();
    static SoundFileManager * instance_;

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILEMANAGER_H
