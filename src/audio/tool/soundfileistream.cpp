/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2015</p>
*/

#include <memory>
#include <cstdio> // for SEEK_SET

#include <sndfile.h>

#include "soundfileistream.h"
#include "io/error.h"
#include "io/log.h"

#if 0
#   define MO_DEBUG_SFIS(arg__) MO_PRINT(arg__)
#else
#   define MO_DEBUG_SFIS(unused__) { }
#endif

namespace MO {
namespace AUDIO {


class SoundFileIStream::Private
{
public:

    /** A pre-read buffer */
    struct Buffer
    {
        size_t pos, len;
        std::vector<F32> buf;
    };


    Private(SoundFileIStream*parent)
        : parent        (parent)
        , sfFile        (0)
        , filePos       (0)
        , readPos       (0)
        , lookAhead     (1<<16)
    { }

    /** Closes file if open and clears all flags and buffers */
    void clear();
    void open(const QString& fn);

    void readBuffer(Buffer*, size_t numSamples);
    size_t read(F32 *buffer, size_t samples);

    SoundFileIStream * parent;
    bool ok;
    QString filename;
    SF_INFO sfInfo;
    SNDFILE * sfFile;
    size_t filePos, readPos, lookAhead;

    std::vector<std::shared_ptr<Buffer>> buffers;
};


SoundFileIStream::SoundFileIStream()
    : p_        (new Private(this))
{

}

SoundFileIStream::~SoundFileIStream()
{
    close();
    delete p_;
}

bool SoundFileIStream::isOpen() const
{
    return p_->sfFile != 0;
}

size_t SoundFileIStream::sampleRate() const
{
    return p_->sfInfo.samplerate;
}

size_t SoundFileIStream::numChannels() const
{
    return p_->sfInfo.channels;
}

size_t SoundFileIStream::lengthSamples() const
{
    return p_->sfInfo.frames;
}

Double SoundFileIStream::lengthSeconds() const
{
    return sampleRate() < 1 ? 0
                            : double(lengthSamples()) / sampleRate();
}

void SoundFileIStream::open(const QString &fn)
{
    p_->open(fn);
}

void SoundFileIStream::close()
{
    p_->clear();
}

void SoundFileIStream::seek(size_t samplePos)
{
    sf_seek(p_->sfFile, samplePos, SEEK_SET);
    p_->filePos = p_->readPos = samplePos;
}

size_t SoundFileIStream::read(F32 *buffer, size_t samples)
{
    return p_->read(buffer, samples);
}



void SoundFileIStream::Private::clear()
{
    MO_DEBUG_SFIS("SoundFileIStream::Private::clear()");

    if (sfFile)
        sf_close(sfFile);
    sfFile = 0;
    filePos = readPos = 0;
    ok = false;
    filename.clear();
    memset(&sfInfo, 0, sizeof(SF_INFO));

    buffers.clear();
}

void SoundFileIStream::Private::open(const QString &fn)
{
    MO_DEBUG_SFIS("SoundFileIStream::Private::open('" << fn << "')");

    clear();

    filename = fn;
    sfFile = sf_open(filename.toStdString().c_str(), SFM_READ, &sfInfo);

    if (!sfFile)
    {
        clear();
        MO_IO_ERROR(READ, "Could not open '" << filename << "'\n"
                    << sf_strerror((SNDFILE*)0));
    }

    if (sfInfo.samplerate < 1)
    {
        clear();
        MO_IO_ERROR(VERSION_MISMATCH, "Invalid sampling rate for file '"
                    << filename << "'");
    }

    MO_DEBUG_SFIS("SoundFileIStream::Private::open() ok : "
                  << parent->sampleRate() << "Hz x "
                  << parent->numChannels() << " chan, "
                  << parent->lengthSamples() << " samples, "
                  << parent->lengthSeconds() << " seconds");
}

void SoundFileIStream::Private::readBuffer(Buffer* b, size_t numSamples)
{
    MO_DEBUG_SFIS("SoundFileIStream::Private::readBuffer("
                  << b << ", " << numSamples << ") "
                  "filePos=" << filePos << ", "
                  "readPos=" << readPos);

    // prepare buffer
    const size_t numFloats = numSamples * parent->numChannels();
    b->pos = filePos;
    if (b->buf.size() < numFloats)
        b->buf.resize(numFloats);
    // read data reading
    b->len = sf_readf_float(sfFile, &b->buf[0], numSamples);
    // forward ptr
    filePos += b->len;
}

size_t SoundFileIStream::Private::read(F32 *buffer, size_t numSamples)
{
    MO_DEBUG_SFIS("SoundFileIStream::Private::read("
                  << buffer << ", " << numSamples << ") "
                  "readPos=" << readPos);
#if 1
    // quick hack

    const size_t len = sf_readf_float(sfFile, buffer, numSamples);
    readPos += len;
    filePos += len;
    // zero rest
    if (len < numSamples)
    {
        const size_t
                s = (numSamples - len) * parent->numChannels(),
                e = numSamples * parent->numChannels();
        for (size_t i = s; i < e; ++i)
            buffer[i] = F32(0);
    }
    return len;

#else
    Buffer * b = 0;
    // create buffer if needed
    if (buffers.empty())
    {
        b = new Buffer();
        b->pos = b->len = 0;
        auto sp = std::shared_ptr<Buffer>(b);
        buffers.push_back(sp);
    }
    else
        // TODO: Multiple buffers...
        b = buffers.front().get();

//    MO_DEBUG_SFIS("b->pos=" << b->pos << ", b->len=" << b->len);

    // see if buffer fulfills request
    if (b->pos > readPos || b->pos + b->len <= readPos + numSamples)
    {
        readBuffer(b, std::max(numSamples, lookAhead));
    }

    // get requested part in pre-read buffer
    std::ptrdiff_t
            start = std::ptrdiff_t(readPos) - std::ptrdiff_t(b->pos),
            len = std::min(std::ptrdiff_t(numSamples),
                           std::ptrdiff_t(b->len) - start);

    memcpy(buffer, &b->buf[start * parent->numChannels()],
            sizeof(F32) * len * parent->numChannels());

    readPos += len;

    return len;
#endif
}



} // namespace AUDIO
} // namespace MO
