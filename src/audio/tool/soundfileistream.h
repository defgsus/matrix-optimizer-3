/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2015</p>
*/

#ifndef MOSRC_AUDIO_TOOL_SOUNDFILEISTREAM_H
#define MOSRC_AUDIO_TOOL_SOUNDFILEISTREAM_H

#include <QString>
#include <types/float.h>

namespace MO {
namespace AUDIO {

/** Input stream for libsndfile compatible files.
    Provides buffered streaming from disk.
    Low-level interface. */
class SoundFileIStream
{
public:
    SoundFileIStream();
    ~SoundFileIStream();

    // ----------- getter --------------

    /** Returns if file is open and readable */
    bool isOpen() const;

    size_t sampleRate() const;
    size_t numChannels() const;
    size_t lengthSamples() const;
    Double lengthSeconds() const;

    // ------- streaming interface -----

    /** Tries to open the file.
        @throws IoException on any error */
    void open(const QString& fn);

    void close();

    void seek(size_t samplePos);

    /** Read the number of samples into @p buffer.
        @p buffer needs to have space for @p samples * numChannels() */
    size_t read(F32 * buffer, size_t samples);

private:

    class Private;
    Private * p_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILEISTREAM_H
