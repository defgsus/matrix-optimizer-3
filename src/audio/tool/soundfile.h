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


class SoundFile
{
public:
    SoundFile();

    // ---------- getter -------------

    /** Returns the filename of the sound file */
    const QString& filename() const { return filename_; }

    /** Returns the sampling rate in Hertz */
    uint sampleRate() const { return sr_; }

    /** Returns length in seconds */
    Double lengthSeconds() const { return lenSec_; }

    /** Returns length in samples */
    uint lengthSamples() const { return lenSam_; }

    /** Returns value at @p time (in seconds) */
    Double value(Double time) const;

private:

    QString filename_;

    uint sr_, lenSam_;
    Double lenSec_;
};


} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_SOUNDFILE_H
