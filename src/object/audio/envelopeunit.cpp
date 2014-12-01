/** @file envelopeunit.cpp

    @brief Envelope follower as AudioUnit

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "envelopeunit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/modulatorobjectfloat.h"
#include "audio/tool/envelopefollower.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(EnvelopeUnit)

EnvelopeUnit::EnvelopeUnit(QObject *parent) :
    AudioUnit(-1, 0, false, parent)
{
    setName("EnvelopeUnit");
}

EnvelopeUnit::~EnvelopeUnit()
{
    for (auto f : follower_)
        delete f;
}

void EnvelopeUnit::serialize(IO::DataStream & io) const
{
    AudioUnit::serialize(io);
    io.writeHeader("auenv", 1);
}

void EnvelopeUnit::deserialize(IO::DataStream & io)
{
    AudioUnit::deserialize(io);
    io.readHeader("auenv", 1);
}

void EnvelopeUnit::createParameters()
{
    AudioUnit::createParameters();

    params()->beginParameterGroup("envset", tr("envelope"));

        amplitude_ = params()->createFloatParameter("amp", tr("amplitude"),
                                     tr("Multiplier for the generated envelope"),
                                     1.0, 0.1);

        fadeIn_ = params()->createFloatParameter("fadein", tr("time up"),
                                     tr("Sets the time to follow rising signals in seconds"),
                                     0.01, 0.001);
        fadeIn_->setMinValue(0.00001);

        fadeOut_ = params()->createFloatParameter("fadeout", tr("time down"),
                                     tr("Sets the time to follow decaying signals in seconds"),
                                     0.5, 0.001);
        fadeOut_->setMinValue(0.00001);

    params()->endParameterGroup();
}

void EnvelopeUnit::channelsChanged()
{
    AudioUnit::channelsChanged();

    createFollowers_();
}

void EnvelopeUnit::setNumberThreads(uint num)
{
    AudioUnit::setNumberThreads(num);

    createFollowers_();
}

void EnvelopeUnit::createFollowers_()
{
    // clear old
    for (auto f : follower_)
        delete f;
    follower_.clear();

    // create new
    const uint num = numChannelsIn() * numberThreads();
    for (uint i = 0; i<num; ++i)
        follower_.push_back( new AUDIO::EnvelopeFollower() );

    // tell scene to create the output ModulatorObjects
    requestCreateOutputs();
}

void EnvelopeUnit::createOutputs()
{
    outputs_.clear();
    for (uint i=0; i<numChannelsIn(); ++i)
    {
        outputs_.push_back( createOutputFloat(
                        QString("env%1_").arg(i+1),
                        QString("env %1").arg(i+1)) );
    }
}

void EnvelopeUnit::processAudioBlock(const F32 *input, Double time, uint thread)
{
    MO_ASSERT(thread < follower_.size(), "thread " << thread << " out of range for "
              "EnvelopeUnit with " << follower_.size() << " threads");

    MO_ASSERT(numChannelsIn() <= outputs_.size(),
              "numChannelsIn (" << numChannelsIn() << ") and "
              "outputs_.size (" << outputs_.size() << ") do not match.");

    const F32 fadeIn = fadeIn_->value(time, thread),
              fadeOut = fadeOut_->value(time, thread),
              amp = amplitude_->value(time, thread);

    const uint bsize = bufferSize(thread);

    for (uint i=0; i<numChannelsIn(); ++i)
    {
        AUDIO::EnvelopeFollower * follower = follower_[i * numberThreads() + thread];

        // adjust filter settings
        if (fadeIn != follower->fadeIn()
            || fadeOut != follower->fadeOut()
            || sampleRate() != follower->sampleRate())
        {
            follower->setFadeIn(fadeIn);
            follower->setFadeOut(fadeOut);
            follower->setSampleRate(sampleRate());
            follower->updateCoefficients();
        }

        outputs_[i]->setValue(time,
                        amp * follower->process(&input[i*bsize], bsize)
                             );

        //MO_DEBUG(QString(" ").repeated(follower->envelope()*50) + "*");
    }
}


} // namespace MO
