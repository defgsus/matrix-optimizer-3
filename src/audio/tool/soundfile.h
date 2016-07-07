/** @file soundfile.h

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SOUNDFILE_H
#define MOSRC_AUDIO_TOOL_SOUNDFILE_H

#include <QString>
#include <QList>

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

    /** Returns an error description, empty string if no error */
    const QString& errorString() const;

    /** Returns true if file was loaded / data can be read */
    bool isOk() const;

    /** Returns true for writeable data */
    bool isWriteable() const;

    /** Returns true if this is an open filestream instead
        of a file-in-memory. */
    bool isStream() const;

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
    Double value(size_t frame, uint channel) const;

    /** Returns one channel as consecutive data */
    std::vector<F32> getSamples(uint channel = 0, uint lengthSamples = 0) const;

    /** Returns one channel as consecutive data, resampled. */
    std::vector<F32> getResampled(uint sampleRate, uint channel = 0, uint lengthSamples = 0) const;

    /** Fills the audiobuffers,
        is aware of NULL buffers in @p channels. */
    void getResampled(const QList<AudioBuffer*> channels,
                      SamplePos frame, uint sampleRate,
                      F32 amplitude = 1.f, F32 pitch = 1.f);

    // --------- setter ---------------

    void appendDeviceData(const F32 * buf, size_t numSamples);

    /** Calls AUDIO::SoundFileManager::releaseSoundFile(this) */
    void release();

    // ------------- io ---------------

    /** Stores a copy of the file */
    void saveFile(const QString& fn) const;

private:

    /** Loads file into memory.
        @throws IoException on errors */
    void p_loadFile_(const QString&);
    void p_openStream_(const QString&);
    void p_create_(uint channels, uint sr, int bitSize);
    void p_setError_(const QString&);

    class Private;
    Private * p_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILE_H
