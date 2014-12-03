/** @file filterao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include "filterao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "audio/tool/multifilter.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(FilterAO)

class FilterAO::Private
{
    public:

    ParameterFloat
        * paramFreq,
        * paramReso;
    ParameterInt
        * paramOrder;
    ParameterSelect
        * paramType;

    std::vector<AUDIO::MultiFilter> filters;
};

FilterAO::FilterAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private())
{
    setName("Filter");
    setNumberAudioOutputs(1);
}

FilterAO::~FilterAO()
{
    delete p_;
}

void FilterAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aoosc", 1);
}

void FilterAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aoosc", 1);
}

void FilterAO::createParameters()
{
    params()->beginParameterGroup("osc", tr("Filter"));

        p_->paramType = params()->createSelectParameter("_filter_type", tr("filter type"),
                                                  tr("Selectes the type of filter"),
                                                  AUDIO::MultiFilter::filterTypeIds,
                                                  AUDIO::MultiFilter::filterTypeNames,
                                                  AUDIO::MultiFilter::filterTypeStatusTips,
                                                  AUDIO::MultiFilter::filterTypeEnums,
                                                  AUDIO::MultiFilter::T_24_LOW, true, false);

        p_->paramOrder = params()->createIntParameter("_filter_order", tr("order"),
                                                   tr("The order (sharpness) of the filter for the 'nth order' types"),
                                                   2,
                                                   1, 10,
                                                   1, true, true);

        p_->paramFreq = params()->createFloatParameter("_filter_freq", tr("frequency"),
                                                   tr("The cutoff frequency of the filter in Hertz"),
                                                   1000.0, 10.0);
        p_->paramReso = params()->createFloatParameter("_filter_reso", tr("resonance"),
                                                   tr("The filter steepness [0,1]"),
                                                   0.0, 0.02);
    params()->endParameterGroup();
}

void FilterAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->filters.resize(count);
}

void FilterAO::processAudio(const QList<AUDIO::AudioBuffer *> &inputs,
                                const QList<AUDIO::AudioBuffer *> &outputs,
                                uint , SamplePos pos, uint thread)
{
    const Double time = sampleRateInv() * pos;

    // update filter
    AUDIO::MultiFilter * filter = &p_->filters[thread];

    Float   freq = p_->paramFreq->value(time, thread),
            res = p_->paramReso->value(time, thread);
    uint    order = p_->paramOrder->value(time, thread);
    auto    type = (AUDIO::MultiFilter::FilterType)
                        p_->paramType->baseValue();

    if (filter->type() != type
            || filter->resonance() != res
            || filter->frequency() != freq
            || filter->order() != order
            || filter->sampleRate() != sampleRate())
    {
        filter->setType(type);
        filter->setFrequency(freq);
        filter->setResonance(res);
        filter->setSampleRate(sampleRate());
        filter->setOrder(order);
        filter->updateCoefficients();
    }

    AUDIO::AudioBuffer::process(inputs, outputs,
    [filter](const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        filter->process(in->readPointer(), out->writePointer(), out->blockSize());
    });
}

} // namespace MO
