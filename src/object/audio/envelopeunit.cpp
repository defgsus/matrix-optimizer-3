/** @file envelopeunit.cpp

    @brief Envelope follower as AudioUnit

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "envelopeunit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/param/parameterfloat.h"
#include "audio/tool/envelopefollower.h"

namespace MO {

MO_REGISTER_OBJECT(EnvelopeUnit)

EnvelopeUnit::EnvelopeUnit(QObject *parent) :
    AudioUnit(0, 0, true, parent)
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
    /*
    processModeParameter_ = createSelectParameter(
                        "processmode", tr("processing"),
                        tr("Sets the processing mode"),
                        { "on", "off", "bypass" },
                        { tr("on"), tr("off"), tr("bypass") },
                        { tr("Processing is always on"),
                          tr("Processing is off, no signals are passed through"),
                          tr("The unit does no processing and passes it's input data unchanged") },
                        { PM_ON, PM_OFF, PM_BYPASS },
                        true, false);
    */

    fadeIn_ = createFloatParameter("fadein", tr("follow speed up"),
                                 tr("Sets the speed to follow rising signals in seconds."),
                                 0.01, 0.001);
    fadeIn_->setMinValue(0.00001);

    fadeOut_ = createFloatParameter("fadeout", tr("follow speed down"),
                                 tr("Sets the speed to follow decaying signals in seconds."),
                                 0.01, 0.001);
    fadeOut_->setMinValue(0.00001);

}

void EnvelopeUnit::channelsChanged(uint thread)
{
    AudioUnit::channelsChanged(thread);

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
}

void EnvelopeUnit::processAudioBlock(const F32 *input, Double time, uint thread)
{
    MO_ASSERT(thread < follower_.size(), "thread " << thread << " out of range for "
              "EnvelopeUnit with " << follower_.size() << " threads");

    const F32 fadeIn = fadeIn_->value(time, thread),
              fadeOut = fadeOut_->value(time, thread);
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

        follower->process(&input[i*bsize], bsize);
    }
}


} // namespace MO
