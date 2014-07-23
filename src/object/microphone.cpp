/** @file microphone.cpp

    @brief basic microphone object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include "microphone.h"
#include "io/datastream.h"
#include "io/error.h"
#include "audio/audiosource.h"

namespace MO {

MO_REGISTER_OBJECT(Microphone)

Microphone::Microphone(QObject *parent) :
    Object(parent)
{
    setName("Microphone");
}

void Microphone::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("mic", 1);
}

void Microphone::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("mic", 1);
}



void Microphone::sampleAudioSource(const AUDIO::AudioSource *src, F32 *buffer, uint thread) const
{
    const uint size = bufferSize(thread);
    MO_ASSERT(size == src->bufferSize(thread), "unmatched buffer size");

    for (uint i=0; i<size; ++i)
    {
        F32 sam = src->getSample(thread, i);

        *buffer++ = sam * 0.3;
    }
}



} // namespace MO
