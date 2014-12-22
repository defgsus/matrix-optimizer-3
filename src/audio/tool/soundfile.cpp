/** @file soundfile.cpp

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include <vector>

#include <sndfile.h>

#include "soundfile.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {


class SoundFile::Private
{
public:

    Private()
        : ok        (false),
          sr        (0),
          channels  (0),
          lenSam    (0),
          lenSec    (0)
    { }

    bool ok;

    QString filename;

    uint sr, channels, lenSam;
    Double lenSec;

    /** 16 for uint16_t, 32 for float */
    uint bitSize;

    std::vector<unsigned char> data;
};



SoundFile::SoundFile()
    : p_    (new Private())
{
    MO_DEBUG_SND("SoundFile::SoundFile()");
}

SoundFile::~SoundFile()
{
    MO_DEBUG_SND("SoundFile::~SoundFile()");

    delete p_;
}

// --------------- getter ------------------------

bool SoundFile::ok() const
{
    return p_->ok;
}

const QString& SoundFile::filename() const
{
    return p_->filename;
}

uint SoundFile::sampleRate() const
{
    return p_->sr;
}

uint SoundFile::numberChannels() const
{
    return p_->channels;
}

Double SoundFile::lengthSeconds() const
{
    return p_->lenSec;
}

uint SoundFile::lengthSamples() const
{
    return p_->lenSam;
}


Double SoundFile::value(Double time, uint channel) const
{
    if (!p_->ok)
        return 0.0;

    // 31bit gives around 13 hours of seekable samples (in mono!)
    const int frame = time * sampleRate();
    if (frame < 0 || frame >= (int)p_->lenSam)
        return 0.0;

    if (p_->bitSize == 16)
    {
        const int16_t * ptr = (const int16_t*)
                &p_->data[(frame * p_->channels + channel) << 1];
        return (Double)*ptr / 32768;
    }
    else
        return 0.0;

}

Double SoundFile::value(uint frame, uint channel) const
{
    if (!p_->ok)
        return 0.0;

    // 31bit gives around 13 hours of seekable samples (in mono!)
    // const int frame = time * sampleRate();
    if (frame < 0 || frame >= (uint)p_->lenSam)
        return 0.0;

    if (p_->bitSize == 16)
    {
        const int16_t * ptr = (const int16_t*)
                &p_->data[(frame * p_->channels + channel) << 1];
        return (Double)*ptr / 32768;
    }
    else
        return 0.0;

}


void SoundFile::loadFile_(const QString & fn)
{
    MO_DEBUG_SND("SoundFile::loadFile_('" << fn << "'");

    p_->ok = false;
    p_->filename = fn;

    // ---- read wave ------


    SF_INFO info;
    info.format = 0;
    SNDFILE *f = sf_open(p_->filename.toStdString().c_str(), SFM_READ, &info);
    if (!f)
    {
        MO_IO_ERROR(READ, "Could not open '" << p_->filename << "'\n"
                    << sf_strerror((SNDFILE*)0));
    }

    if (info.samplerate < 1)
    {
        sf_close(f);
        MO_IO_ERROR(VERSION_MISMATCH, "Invalid sampling rate for file '"
                    << p_->filename << "'");
    }

    // copy values

    p_->channels = info.channels;
    p_->sr = info.samplerate;
    p_->lenSam = info.frames;
    p_->lenSec = (Double)info.frames / info.samplerate;

    // init storage

    p_->bitSize = 16;
    p_->data.resize(p_->channels * p_->lenSam * 2);

    uint e = sf_readf_short(f, (int16_t*)&p_->data[0], p_->lenSam);

    if (e != p_->lenSam)
    {
        sf_close(f);
        MO_IO_ERROR(READ, "could not load all of soundfile '" << p_->filename << "'\n"
                    "expected " << p_->lenSam << " frames, got " << e );
    }

    p_->ok = true;

    MO_DEBUG_SND("SoundFile::loadFile_() finished, "
                 << p_->channels << "ch. "
                 << p_->bitSize << "bit @ "
                 << p_->sr << "hz, frames=" << p_->lenSam
                 << ", secs=" << p_->lenSec);

    sf_close(f);
}

} // namespace AUDIO
} // namespace MO
