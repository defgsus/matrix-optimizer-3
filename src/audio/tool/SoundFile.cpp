/** @file soundfile.cpp

    @brief Wrapper around audio files

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/24/2014</p>
*/

#include <vector>

#include <sndfile.h>

#include "SoundFile.h"
#include "SoundFileManager.h"
#include "audio/Configuration.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/tool/SoundFileIStream.h"
#include "math/interpol.h"
#include "math/functions.h"
#include "io/error.h"
#include "io/log_snd.h"

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
          lenSec    (0),
          stream    (0)
    { }

    ~Private()
    {
        delete stream;
    }

    bool ok, writeable, isStream;

    QString filename, errorStr;

    uint sr, channels, lenSam;
    Double lenSec;

    /** 16 for uint16_t, 32 for float */
    uint bitSize;

    SoundFileIStream * stream;

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

void SoundFile::release()
{
    AUDIO::SoundFileManager::releaseSoundFile(this);
}

void SoundFile::addRef()
{
    AUDIO::SoundFileManager::addReference(this);
}

// --------------- getter ------------------------

const QString& SoundFile::errorString() const
{
    return p_->errorStr;
}

QString SoundFile::infoString() const
{
    return QString("%1ch, %2Hz, %3bits, %4secs, %5sams, %6")
            .arg(numberChannels())
            .arg(sampleRate())
            .arg(p_->bitSize)
            .arg(lengthSeconds())
            .arg(lengthSamples())
            .arg(filename());
}

bool SoundFile::isOk() const
{
    return p_->ok;
}

bool SoundFile::isWriteable() const
{
    return p_->writeable;
}

bool SoundFile::isStream() const
{
    return p_->stream;
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

std::vector<F32> SoundFile::getSamples(uint channel, uint len) const
{
    if (!isOk() || numberChannels() == 0)
        return std::vector<F32>();

    if (len == 0)
        len = lengthSamples();

    if (channel >= numberChannels())
        channel = numberChannels() - 1;

    std::vector<F32> ret(len);
    F32 * dst = &ret[0];

    if (isStream())
    {
        /** @todo SoundFile::getSamples(channel, len): implement read from SoundFileIStream */
        //p_->stream->readChannel()

        return ret;
    }

    if (p_->bitSize == 16)
        for (size_t i=0; i<len; ++i)
            *dst++ = F32(*((int16_t*)&p_->data[i*2*p_->channels + channel])) / 32768.f;
    else
    if (p_->bitSize == 32)
        for (size_t i=0; i<len; ++i)
            *dst++ = *((F32*)&p_->data[i*4*p_->channels + channel]);

    return ret;
}

std::vector<F32> SoundFile::getResampled(uint sr, uint channel, uint len) const
{
    if (!isOk() || numberChannels() == 0)
        return std::vector<F32>();

    if (len == 0)
        len = Double(lengthSamples()) / sampleRate() * sr;

    if (channel >= numberChannels())
        channel = numberChannels() - 1;

    std::vector<F32> ret(len);
    for (size_t i=0; i<len; ++i)
        ret[i] = value(Double(i) / sampleRate(), channel);

    return ret;
}

void SoundFile::getResampled(
        const QList<AudioBuffer*> channels,
        SamplePos frame, uint sr, F32 amp, F32 pitch)
{
    if (channels.isEmpty())
        return;

    const Double
            invSr = 1.0 / sr,
            invSrThis = 1. / std::max((uint)1, sampleRate());
    size_t i, num = std::min((int)numberChannels(), channels.size());

    // read from stream
    if (isStream())
    {
        size_t bsize = channels.front()->blockSize();
        F32 buf[bsize * numberChannels()];

        p_->stream->seek(frame);
        p_->stream->read(buf, bsize);

        for (i=0; i<num; ++i)
        {
            if (!channels[i])
                continue;

            // XXX no resampling
            for (size_t j=0; j<bsize; ++j)
            {
                channels[i]->write(j, buf[j*numberChannels() + i] * amp);
            }
        }

        return;
    }

    // read from memory
    for (i=0; i<num; ++i)
    {
        if (!channels[i])
            continue;

        // XXX poor eff.
        for (size_t j=0; j<channels[i]->blockSize(); ++j)
        {
            Double time = Double(frame) * invSrThis + Double(j) * invSr * pitch;
            channels[i]->write(j, value(time, i) * amp);
        }
    }
}


Double SoundFile::value(Double time, uint channel, bool interpol) const
{
    if (!p_->ok)
        return 0.0;

    // 31bit gives around 13 hours of seekable samples (in mono!)
    const long int frame = time * sampleRate();
    if (frame < 0 || frame >= (long int)p_->lenSam)
        return 0.0;

    if (isStream())
    {
        /** @todo super hacky inefficient  */
        F32 buf[numberChannels()];
        p_->stream->seek(frame);
        p_->stream->read(buf, 1);
        return buf[channel];
    }


    if (p_->bitSize == 16)
    {
        if (!interpol)
        {
            const int16_t * ptr =
                    (const int16_t*)&p_->data[(frame * p_->channels + channel) << 1];
            return (Double)*ptr / 32768.;

        }
        else
        {
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
        }
    }
    else if (p_->bitSize == 32)
    {
        if (!interpol)
        {
            const F32 * ptr =
                (const F32*)&p_->data[(frame * p_->channels + channel) << 2];
            return (Double)*ptr;
        }
        else
        {
            const long int fs = p_->channels * 4;
            size_t pos = (frame * p_->channels + channel) * 4;
            Double v0,v1,v2,v3,v4,v5;

            v0 = frame > 1 ? Double(*((const F32*)&p_->data[pos-fs*2])) : 0.;
            v1 = frame > 0 ? Double(*((const F32*)&p_->data[pos-fs])) : 0.;
            v2 = Double(*((const F32*)&p_->data[pos]));
            v3 = frame < (long int)p_->lenSam - 1 ? Double(*((const F32*)&p_->data[pos+fs])) : 0.;
            v4 = frame < (long int)p_->lenSam - 2 ? Double(*((const F32*)&p_->data[pos+fs*2])) : 0.;
            v5 = frame < (long int)p_->lenSam - 3 ? Double(*((const F32*)&p_->data[pos+fs*3])) : 0.;

            Double t = MATH::fract(time * sampleRate());
            return MATH::interpol_6(t, v0,v1, v2, v3,v4,v5);
        }
    }
    else
        return 0.0;

}

Double SoundFile::value(size_t frame, uint channel) const
{
    if (!p_->ok || channel >= numberChannels())
        return 0.0;

    // 31bit gives around 13 hours of seekable samples (in mono!)
    // const int frame = time * sampleRate();
    if (frame >= p_->lenSam)
        return 0.0;

    // read from stream
    if (isStream())
    {
        /** @todo super hacky inefficient  */
        F32 buf[numberChannels()];
        p_->stream->seek(frame);
        p_->stream->read(buf, 1);
        return buf[channel];
    }

    // read from memory
    if (p_->bitSize == 16)
    {
        const int16_t * ptr = (const int16_t*)
                &p_->data[(frame * p_->channels + channel) << 1];
        return ((Double)*ptr) / 32768.;
    }
    else if (p_->bitSize == 32)
    {
        const F32 * ptr = (const F32*)
                &p_->data[(frame * p_->channels + channel) << 2];
        return (Double)*ptr;
    }
    else
        return 0.0;

}



void SoundFile::appendDeviceData(const F32 *buf, size_t numSamples)
{
    if (!isWriteable())
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


void SoundFile::p_setError_(const QString &e) { p_->errorStr = e; }
void SoundFile::p_setFilename_(const QString &e) { p_->filename = e; }


void SoundFile::p_create_(uint channels, uint sr, int bitSize)
{
    p_->bitSize = bitSize;
    p_->channels = channels;
    p_->sr = sr;
    p_->lenSam = 0;
    p_->lenSec = 0;
    p_->data.clear();
    p_->writeable = true;
    p_->ok = true;
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


#if 0
    // init storage
    p_->bitSize = 16;
    p_->data.resize(p_->channels * p_->lenSam * 2);

    uint e = sf_readf_short(f, (int16_t*)&p_->data[0], p_->lenSam);
#else
    p_->bitSize = 32;
    p_->data.resize(p_->channels * p_->lenSam * 4);

    uint e = sf_readf_float(f, (F32*)&p_->data[0], p_->lenSam);
#endif

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

void SoundFile::p_openStream_(const QString& fn)
{
    p_->ok = false;
    p_->filename = fn;

    if (!p_->stream)
        p_->stream = new SoundFileIStream();
    p_->stream->open(fn);

    p_->ok = true;
    p_->channels = p_->stream->numChannels();
    p_->sr = p_->stream->sampleRate();
    p_->bitSize = 32;
    p_->lenSam = p_->stream->lengthSamples();
    p_->lenSec = p_->stream->lengthSeconds();
}

void SoundFile::saveFile(const QString &fn) const
{
    MO_DEBUG_SND("SoundFile::saveFile('" << fn << "')");

    if (p_->stream)
        MO_IO_ERROR(WRITE, "Can't save audio stream file");

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
