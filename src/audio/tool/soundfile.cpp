/** @file soundfile.cpp

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include <vector>

#include <sndfile.h>

#include "soundfile.h"
#include "audio/configuration.h"
#include "math/interpol.h"
#include "math/functions.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {


class SoundFile::Private
{
public:

    Private()
        : ok        (false),
          writeable (false),
          sr        (0),
          channels  (0),
          lenSam    (0),
          lenSec    (0)
    { }

    bool ok, writeable;

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

bool SoundFile::writeable() const
{
    return p_->writeable;
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
    const long int frame = time * sampleRate();
    if (frame < 0 || frame >= (long int)p_->lenSam)
        return 0.0;

    if (p_->bitSize == 16)
    {
#if 0
        const int16_t * ptr =
                (const int16_t*)&p_->data[(frame * p_->channels + channel) << 1];
        return (Double)*ptr / 32768;
#else
        const long int fs = p_->channels *2;
        size_t pos = (frame * p_->channels + channel) * 2;
        Double v0,v1,v2,v3,v4,v5;

        v0 = frame > 1 ? Double(*((const int16_t*)&p_->data[pos-fs*2])) / 32768. : 0.;
        v1 = frame > 0 ? Double(*((const int16_t*)&p_->data[pos-fs])) / 32768. : 0.;
        v2 = Double(*((const int16_t*)&p_->data[pos])) / 32768.;
        v3 = frame < (long int)p_->lenSam - 1 ? Double(*((const int16_t*)&p_->data[pos+fs])) / 32768. : 0.;
        v4 = frame < (long int)p_->lenSam - 2 ? Double(*((const int16_t*)&p_->data[pos+fs*2])) / 32768. : 0.;
        v5 = frame < (long int)p_->lenSam - 3 ? Double(*((const int16_t*)&p_->data[pos+fs*3])) / 32768. : 0.;

        Double t = MATH::fract(time * sampleRate());
        return MATH::interpol_6(t, v0,v1, v2, v3,v4,v5);
#endif
    }
    else
        return 0.0;

}



void SoundFile::appendDeviceData(const F32 *buf, size_t numSamples)
{
    if (!writeable())
        return;

    if (p_->bitSize == 16)
    {
        size_t pos = p_->data.size();

        p_->data.resize(pos + (numSamples * p_->channels * 2));

        for (size_t i=0; i<numSamples * p_->channels; ++i)
        {
            *((int16_t*)&p_->data[pos + i*2]) = buf[i] * 32767;
        }
    }
    else
    if (p_->bitSize == 32)
    {
        size_t pos = p_->data.size();

        p_->data.resize(pos + (numSamples * p_->channels * 4));

        for (size_t i=0; i<numSamples * p_->channels; ++i)
        {
            *((F32*)&p_->data[pos + i*4]) = buf[i];
        }
    }

    p_->lenSam += numSamples;
    p_->lenSec = Double(p_->lenSam) / p_->sr;
}




void SoundFile::p_create_(uint channels, uint sr, int bitSize)
{
    p_->bitSize = bitSize;
    p_->channels = channels;
    p_->sr = sr;
    p_->lenSam = 0;
    p_->lenSec = 0;
    p_->data.clear();
    p_->writeable = true;
}


void SoundFile::p_loadFile_(const QString & fn)
{
    MO_DEBUG_SND("SoundFile::loadFile_('" << fn << "')");

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


void SoundFile::saveFile(const QString &fn) const
{
    MO_DEBUG_SND("SoundFile::saveFile('" << fn << "')");

    SF_INFO info;
    info.channels = p_->channels;
    info.samplerate = p_->sr;
    info.frames = p_->lenSam;
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE *f = sf_open(fn.toStdString().c_str(), SFM_WRITE, &info);
    if (!f)
        MO_IO_ERROR(WRITE, "could not open file for writing audio '" << fn << "'\n"
                    << sf_strerror((SNDFILE*)0));


    uint e = sf_writef_float(f, (const F32*)&p_->data[0], p_->lenSam);
    sf_close(f);

    if (e != p_->lenSam)
    {
        MO_IO_ERROR(WRITE, "could not write all of soundfile '" << fn << "'\n"
                    "expected " << p_->lenSam << " frames, got " << e );
    }
}



} // namespace AUDIO
} // namespace MO
