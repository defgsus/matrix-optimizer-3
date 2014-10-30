/** @file oscillatorobject.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.10.2014</p>
*/

#include "oscillatorobject.h"
#include "io/datastream.h"
#include "audio/audiosource.h"
#include "param/parameterfloat.h"
#include "io/log.h"


namespace MO {
#ifdef XXX_NEEDS_RETHINKING

MO_REGISTER_OBJECT(OscillatorObject)

OscillatorObject::OscillatorObject(QObject *parent) :
    Object(parent)
{
    setName("OscillatorObject");
}

void OscillatorObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("osco", 1);
}

void OscillatorObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("osco", 1);
}

void OscillatorObject::createParameters()
{
    Object::createParameters();

    beginParameterGroup("", tr("audio"));

        audioTrack_ = createFloatParameter("audio_track", tr("audio"),
                                           tr("The audio signal of the sound source"),
                                           0.0, false);
    endParameterGroup();
}

void OscillatorObject::createAudioSources()
{
    Object::createAudioSources();

    audio_ = createAudioSource();
}

void OscillatorObject::updateAudioTransformations(Double, uint thread)
{
    audio_->setTransformation(transformation(thread, 0), thread, 0);
}

void OscillatorObject::updateAudioTransformations(Double , uint /*size*/, uint thread)
{
#if (1)
    // copy the block of transformations
    audio_->setTransformation(transformations(thread), thread);
#else
    for (uint i=0; i<size; ++i)
    {
        const Float t = Float(i) / (size-1);
        audio_->setTransformation(
                    transformation(thread, 0)
                    + t * (transformation(thread, size-1) - transformation(thread, 0))
                    , thread, i);
    }
#endif
}

void OscillatorObject::performAudioBlock(SamplePos pos, uint thread)
{
    // copy the samples from the audiotrack

    audioTrack_->getValues(pos, thread, sampleRateInv(), bufferSize(thread),
                           audio_->samples(thread));


    /*
    for (uint i=0; i<bufferSize(thread); ++i)
    {
        const int sec = (pos + i) / sampleRate();
        const int sam = (pos + i) % sampleRate();
        audio_->setSample(0.5*sin((sec + sam * sampleRateInv())*6.28*437.0), thread, i);
    }
    */

    /*
    static SamplePos lastpos = 0;
    if (pos - lastpos != bufferSize(thread))
        MO_DEBUG("error " << lastpos << " - " << pos << " = " << (pos - lastpos));
    lastpos = pos;
    */
}
#endif

} // namespace MO
