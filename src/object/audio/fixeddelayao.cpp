/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include "fixeddelayao.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/fixedblockdelay.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(FixedDelayAO)

class FixedDelayAO::Private
{
    public:

    Private(FixedDelayAO * ao)
        : ao            (ao)
        , curDelaySam   (0.f)
    { }

    ParameterFloat
        * paramTime;
    ParameterInt
        * paramSamples;

    FixedDelayAO * ao;
    std::vector<AUDIO::FixedBlockDelay<F32>> delays;
    F32 curDelaySam;
};

FixedDelayAO::FixedDelayAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("FixedDelay");
    setNumberChannelsAdjustable(true);
}

FixedDelayAO::~FixedDelayAO()
{
    delete p_;
}

QString FixedDelayAO::infoString() const
{
    return tr("delay (samples) %1")
            .arg(p_->curDelaySam);
}

void FixedDelayAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofdly", 1);
}

void FixedDelayAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofdly", 1);
}

void FixedDelayAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("_delay_", tr("delay"));
    initParameterGroupExpanded("_delay_");

        p_->paramTime = params()->createFloatParameter(
                    "_delay_time", tr("delay time in seconds"),
                   tr("The current delay time in seconds"),
                    1.0, 0.05, true, false);
        p_->paramTime->setMinValue(0.);

        p_->paramSamples = params()->createIntParameter(
                    "_delay_samples", tr("delay time in samples"),
                    tr("Additional delaytime in samples, added to the seconds"),
                    0, true, false);
        p_->paramSamples->setMinValue(0);

    params()->endParameterGroup();
}

void FixedDelayAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    // nothing
}

void FixedDelayAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->delays.resize(count);
}

void FixedDelayAO::processAudio(const RenderTime& time)
{
    p_->curDelaySam = p_->paramTime->value(time) * sampleRate()
                      + p_->paramSamples->value(time);

    AUDIO::FixedBlockDelay<F32> * delay = &p_->delays[time.thread()];
    if (delay->size() != time.bufferSize()
     || delay->delay() != p_->curDelaySam)
        delay->setSize(time.bufferSize(), p_->curDelaySam);

    AUDIO::AudioBuffer::process(
                audioInputs(time.thread()), audioOutputs(time.thread()),
    [=](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        delay->write(in->readPointer());
        delay->read(out->writePointer());
    });
}

} // namespace MO

