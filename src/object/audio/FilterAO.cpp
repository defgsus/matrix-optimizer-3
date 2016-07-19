/** @file filterao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#include <memory>

#include "FilterAO.h"
#include "audio/tool/AudioBuffer.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "object/param/ParameterCallback.h"
#include "math/constants.h"
#include "audio/tool/MultiFilter.h"
#include "io/DataStream.h"

namespace MO {

MO_REGISTER_OBJECT(FilterAO)

class FilterAO::Private
{
    public:

    void updateFilters();

    ParameterFloat
        * paramFreq,
        * paramReso,
        * paramAmp;
    ParameterInt
        * paramOrder;
    ParameterSelect
        * paramType;
    ParameterCallback
        * paramReset;

    std::vector<std::shared_ptr<AUDIO::MultiFilter>> filters;
    bool doReset;
};

FilterAO::FilterAO()
    : AudioObject   (),
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
    AudioObject::createParameters();

    params()->beginParameterGroup("filter", tr("Filter"));
    initParameterGroupExpanded("filter");

        p_->paramReset = params()->createCallbackParameter(
                "reset", tr("reset"),
                tr("Resets the filter (useful in case of internal overflow"),
                [=](){ p_->doReset = true; });

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
                                                   1, true, false);

        p_->paramFreq = params()->createFloatParameter("_filter_freq", tr("frequency"),
                                                   tr("The cutoff frequency of the filter in Hertz"),
                                                   1000.0, 10.0);
        p_->paramReso = params()->createFloatParameter("_filter_reso", tr("resonance"),
                                                   tr("The filter steepness [0,1]"),
                                                   0.0, 0.0, 1.0, 0.02);
        p_->paramAmp = params()->createFloatParameter("_filter_amp", tr("amplitude"),
                                                   tr("The output amplitude of the filter"),
                                                   1.0, 0.05);

    params()->endParameterGroup();
}

void FilterAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    auto type = (AUDIO::MultiFilter::FilterType)p_->paramType->baseValue();

    p_->paramOrder->setVisible(
                AUDIO::MultiFilter::supportsOrder(type));
    p_->paramReso->setVisible(
                AUDIO::MultiFilter::supportsResonance(type));
}

void FilterAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    // Note: We change the filter type here
    // to avoid memory allocation in the audio thread!
    // Same goes for order
    if (p == p_->paramType || p == p_->paramOrder)
        for (auto & fp : p_->filters)
        {
            auto f = fp.get();
            f->setType((AUDIO::MultiFilter::FilterType)p_->paramType->baseValue());
            f->setOrder(p_->paramOrder->baseValue());
            f->updateCoefficients();
        }
}

void FilterAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

    p_->updateFilters();
}

void FilterAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

    p_->filters.clear();
    for (uint i=0; i<count; ++i)
        p_->filters.push_back( std::shared_ptr<AUDIO::MultiFilter>(
                                   new AUDIO::MultiFilter(true) ));
    p_->updateFilters();
}

void FilterAO::Private::updateFilters()
{
    for (auto & fp : filters)
    {
        auto f = fp.get();
        // set type and order
        // other parameters can/will be set in audio thread
        f->setType((AUDIO::MultiFilter::FilterType)paramType->baseValue());
        f->setOrder(paramOrder->baseValue());
        f->updateCoefficients();
    }
}

void FilterAO::processAudio(const RenderTime& time)
{   
    // update filter
    AUDIO::MultiFilter * filter = p_->filters[time.thread()].get();

    p_->paramReset->fireIfInput(time);
    if (p_->doReset)
    {
        p_->doReset = false;
        filter->reset();
    }

    Float   freq = p_->paramFreq->value(time),
            res = p_->paramReso->value(time),
            amp = p_->paramAmp->value(time);

    // update filter settings
    // XXX Note that these are only updated at the beginning of one dsp block!
    if (   filter->resonance() != res
        || filter->frequency() != freq
        || filter->sampleRate() != sampleRate())
    {
        filter->setFrequency(freq);
        filter->setResonance(res);
        filter->setSampleRate(sampleRate());
        filter->updateCoefficients();
    }
    // does not need updateCoefficients()
    filter->setOutputAmplitude(amp);

    AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
    [filter](uint, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        filter->process(in->readPointer(), out->writePointer(), out->blockSize());
    });
}

} // namespace MO
