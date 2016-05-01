/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/27/2016</p>
*/

#include "counterao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "io/datastream.h"
#include "io/error.h"


namespace MO {

MO_REGISTER_OBJECT(CounterAO)

class CounterAO::Private
{
    public:

    Private(CounterAO * ao) : ao(ao) { }

    CounterAO * ao;

    ParameterFloat
        * paramThresh;
    ParameterInt
        * paramModulo;

    struct Count
    {
        Count() : count(0), wasAbove(false) { }
        int count;
        bool wasAbove;
    };

    // per thread per channel
    std::vector<std::vector<Count>> counts;
};



CounterAO::CounterAO()
    : AudioObject   (),
      p_            (new Private(this))
{
    setName("Counter");
    setNumberChannelsAdjustable(true);
}

CounterAO::~CounterAO()
{
    delete p_;
}

void CounterAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aocount", 1);
}

void CounterAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aocount", 1);
}

void CounterAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("counter", tr("counter"));
    initParameterGroupExpanded("counter");

        p_->paramThresh = params()->createFloatParameter(
                "threshold", tr("input threshold"),
                tr("The threshold above which a raising edge is counted "
                   "as a gate"),
                0.05, 0.05);
        p_->paramModulo = params()->createIntParameter(
                    "modulo", tr("modulo"),
                    tr("If > 0, the counter is take modulo this value"),
                    0, true, true);

        params()->endParameterGroup();
}

void CounterAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();
}

void CounterAO::onParameterChanged(Parameter *p)
{
    AudioObject::onParameterChanged(p);
}

void CounterAO::setNumberThreads(uint num)
{
    AudioObject::setNumberThreads(num);

    p_->counts.resize(num);
}

void CounterAO::setAudioBuffers(uint thread, uint /*bufferSize*/,
                               const QList<AUDIO::AudioBuffer *> &inputs,
                               const QList<AUDIO::AudioBuffer *> &/*outputs*/)
{
    p_->counts[thread].resize(inputs.size());
}

void CounterAO::processAudio(const RenderTime& time)
{
    F32 thresh = p_->paramThresh->value(time);
    int modulo = p_->paramModulo->value(time);

    AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
    [=](uint channel, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        Private::Count* count  = &p_->counts[time.thread()][channel];

        for (size_t i = 0; i<in->blockSize(); ++i)
        {
            double t = in->read(i);
            if (t <= thresh)
                count->wasAbove = false;
            else if (!count->wasAbove)
            {
                count->wasAbove = true;
                ++count->count;
                if (modulo > 0)
                    count->count %= modulo;
            }
            out->write(i, count->count);
        }
    });
}


} // namespace MO
