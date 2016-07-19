/** @file audioplayerdata.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.05.2015</p>
*/

#include "AudioPlayerData.h"
#include "tool/SoundFile.h"

namespace MO {
namespace AUDIO {


Double AudioPlayerData::lengthSeconds() const
{
    return Double(lengthSamples()) / std::max(size_t(1), sampleRate());
}

void AudioPlayerData::seek(Double second)
{
    seek(second * sampleRate());
}

size_t AudioPlayerData::getInterlaced(F32 *buffer, size_t numChan, size_t blockSize)
{
    std::vector<F32> tmp(blockSize * numChan);
    F32 * src = &tmp[0];

    size_t r = get(src, numChan, blockSize);

    for (size_t i=0; i<numChan; ++i)
    {
        F32 * b = buffer;
        for (size_t j=0; j<r; ++j, b += numChan)
            *b = *src++;
        // advance channel
        ++buffer;
    }

    return r;
}




AudioPlayerSample::AudioPlayerSample(size_t length, size_t numChannels, size_t sampleRate)
    : p_len_        (length)
    , p_chan_       (numChannels)
    , p_rate_       (sampleRate)
    , p_pos_        (0)
    , p_data_       (size(), 0.f)
{

}

AudioPlayerSample * AudioPlayerSample::fromData(const F32 * data, size_t length, size_t sampleRate)
{
    auto s = new AudioPlayerSample(length, 1, sampleRate);
    memcpy(s->samples(), data, s->size() * sizeof(F32));
    return s;
}

size_t AudioPlayerSample::get(F32 *buffer, size_t numChan, size_t blockSize)
{
    /*
    size_t i;
    for (i=0; i<blockSize; ++i)
    {
        if (p_pos_++ >= p_len_)
            break;
        F32 t = F32(p_pos_) / 44100.f;
        for (size_t j=0; j<numChan; ++j)
            buffer[i + j * blockSize] = sin(t * 6.28f * 440.f) * (1.f - t);
    }
    return i;*/

    // interlaced -> deinterlaced
    for (size_t i=0; i<blockSize; ++i)
    {
        if (p_pos_ >= p_len_)
            return i;

        for (size_t j=0; j<numChan; ++j)
            buffer[i + j * blockSize] = p_data_[p_pos_ * numChannels() + j];

        ++p_pos_;
    }
    return blockSize;
}

size_t AudioPlayerSample::getInterlaced(F32 *buffer, size_t numChan, size_t blockSize)
{
    // copy interlaced -> interlaced
    for (size_t i=0; i<blockSize; ++i)
    {
        if (p_pos_ >= p_len_)
            return i;

        F32 * src = &p_data_[p_pos_ * numChannels()];
        for (size_t j = 0; j<numChan; ++j)
            *buffer++ = *src++;

        ++p_pos_;
    }
    return blockSize;
}





// #################### AudioPlayerSoundFile ########################

AudioPlayerSoundFile::AudioPlayerSoundFile(SoundFile* sf)
    : p_sf_     (sf)
    , p_pos_    (0)
{
    p_sf_->addRef();
}

AudioPlayerSoundFile::~AudioPlayerSoundFile()
{
    p_sf_->release();
}

size_t AudioPlayerSoundFile::numChannels() const { return p_sf_->numberChannels(); }
size_t AudioPlayerSoundFile::sampleRate() const { return p_sf_->sampleRate(); }
size_t AudioPlayerSoundFile::lengthSamples() const { return p_sf_->lengthSamples(); }

size_t AudioPlayerSoundFile::get(F32 *buffer, size_t numChan, size_t blockSize)
{
    for (size_t i=0; i<blockSize; ++i)
    {
        if (p_pos_ >= p_sf_->lengthSamples())
            return i;

        for (size_t j=0; j<numChan; ++j)
            buffer[i + j * blockSize] = p_sf_->value(p_pos_, j);

        ++p_pos_;
    }
    return blockSize;
}

size_t AudioPlayerSoundFile::getInterlaced(
        F32 *buffer, size_t numChan, size_t blockSize)
{
    for (size_t i=0; i<blockSize; ++i)
    {
        if (p_pos_ >= p_sf_->lengthSamples())
            return i;

        for (size_t j = 0; j<numChan; ++j)
            *buffer++ = p_sf_->value(p_pos_, j);

        ++p_pos_;
    }
    return blockSize;
}

} // namespace AUDIO
} // namespace MO
