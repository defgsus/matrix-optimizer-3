/** @file soundfile.h

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SOUNDFILE_H
#define MOSRC_AUDIO_TOOL_SOUNDFILE_H

#include <QString>

#include <types/float.h>
#include <types/int.h>

namespace MO {
namespace AUDIO {

class AudioBuffer;
class Configuration;

/** This class is a getter in itself.
    Use SoundFileManager to load files */
class SoundFile
{
    friend class SoundFileManager;

    SoundFile();
    ~SoundFile();

public:

    // ---------- getter -------------

    /** Returns true if file was loaded / data can be read */
    bool ok() const;

    /** Returns true for writeable data */
    bool writeable() const;

    /** Returns the filename of the sound file */
    const QString& filename() const;

    /** Returns the sampling rate in Hertz */
    uint sampleRate() const;

    /** Returns the number of channels in the sound file */
    uint numberChannels() const;

    /** Returns length in seconds */
    Double lengthSeconds() const;

    /** Returns length in samples */
    uint lengthSamples() const;

    /** Returns value at @p time (in seconds) */
    Double value(Double time, uint channel = 0) const;

    /** Returns one channel as consecutive data */
    std::vector<F32> getSamples(uint channel = 0, uint lengthSamples = 0) const;

    // --------- setter ---------------

    void appendDeviceData(const F32 * buf, size_t numSamples);

    // ------------- io ---------------

    /** Stores a copy of the file */
    void saveFile(const QString& fn) const;

private:

    /** Loads file into memory.
        @throws IoException on errors */
    void p_loadFile_(const QString&);
    void p_create_(uint channels, uint sr, int bitSize);

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILE_H
