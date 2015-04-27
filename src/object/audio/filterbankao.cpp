/** @file filterbankao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#include <memory>

#include "filterbankao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "audio/tool/multifilter.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(FilterBankAO)

class FilterBankAO::Private
{
    public:

    Private(FilterBankAO * ao) : ao(ao) { }
    FilterBankAO * ao;

    void makeFilters(uint num);
    void updateFilterCoeffs(Double time, uint thread);

    ParameterFloat
        * paramFreqStart,
        * paramFreqAdd,
        * paramFreqMul,
        * paramReso,
        * paramAmp;
        //* paramReset;
    ParameterInt
        * paramOrder;
    ParameterSelect
        * paramType;

    // filters for all channels
    struct Filters
    {
        std::vector<std::shared_ptr<AUDIO::MultiFilter>> filters;
    };

    // per thread
    std::vector<Filters> filters;
};

FilterBankAO::FilterBankAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("FilterBank");
    setNumberChannelsAdjustable(true);
}

FilterBankAO::~FilterBankAO()
{
    delete p_;
}

void FilterBankAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofbank", 1);
}

void FilterBankAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofbank", 1);
}

void FilterBankAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("filter", tr("Filter"));
    initParameterGroupExpanded("filter");

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

        p_->paramFreqStart = params()->createFloatParameter("_filter_freq_start", tr("start frequency"),
                                                   tr("The first frequency of the filter in Hertz"),
                                                   100.0, 10.0);
        p_->paramFreqAdd = params()->createFloatParameter("_filter_freq_add", tr("+ frequency"),
                                                   tr("The frequency to add to the start frequency for each channel in Hertz"),
                                                   0.0, 10.0);
        p_->paramFreqMul = params()->createFloatParameter("_filter_freq_mul", tr("* frequency"),
                                                   tr("The factor to multiply the current frequency for the next channel"),
                                                   1.0, 0.0625);

        p_->paramReso = params()->createFloatParameter("_filter_reso", tr("resonance"),
                                                   tr("The filter steepness [0,1]"),
                                                   0.0, 0.02);
        p_->paramAmp = params()->createFloatParameter("_filter_amp", tr("amplitude"),
                                                   tr("The output amplitude of the filter"),
                                                   1.0, 0.05);
        //p_->paramReset = params()->createGateParameter
    params()->endParameterGroup();
}

void FilterBankAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    auto type = (AUDIO::MultiFilter::FilterType)p_->paramType->baseValue();

    p_->paramOrder->setVisible(
                AUDIO::MultiFilter::supportsOrder(type));
    p_->paramReso->setVisible(
                AUDIO::MultiFilter::supportsResonance(type));
}

void FilterBankAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

    // Note: We change the filter type here
    // to avoid memory allocation in the audio thread!
    // Same goes for order
    /*
    if (p == p_->paramType || p == p_->paramOrder)
        for (auto & f : p_->filters)
        {
            f.setType((AUDIO::MultiFilter::FilterType)p_->paramType->baseValue());
            f.setOrder(p_->paramOrder->baseValue());
            f.updateCoefficients();
        }
        */
}

void FilterBankAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

//    p_->updateFilters();
}

void FilterBankAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

//    p_->makeFilters();
}

void FilterBankAO::Private::makeFilters(uint num)
{
    // filters for each thread
    filters.resize(ao->numberThreads());
    for (auto & fs : filters)
    {
        fs.filters.clear();
        // filter for each channel
        for (uint i=0; i<num; ++i)
            fs.filters.push_back(std::shared_ptr<AUDIO::MultiFilter>(
                                     new AUDIO::MultiFilter(true) ));
    }
}

void FilterBankAO::Private::updateFilterCoeffs(Double time, uint thread)
{
    Float   freq = paramFreqStart->value(time, thread),
            freqAdd = paramFreqAdd->value(time, thread),
            freqMul = paramFreqMul->value(time, thread),
            res = paramReso->value(time, thread),
            amp = paramAmp->value(time, thread);
    int     order = paramOrder->value(time, thread);
    auto    type = (AUDIO::MultiFilter::FilterType)paramType->baseValue();

    // each filter per channel
    for (auto & fp : filters[thread].filters)
    {
        auto f = fp.get();

        // test for changes (to avoid updateCoefficients())
        if (
               f->frequency() != freq
            || f->resonance() != res
            || f->sampleRate() != ao->sampleRate()
            || (int)f->order() != order
            || f->type() != type)
        {
            f->setFrequency(freq);
            f->setResonance(res);
            f->setSampleRate(ao->sampleRate());
            f->setType(type);
            f->setOrder(order);
            f->updateCoefficients();
        }

        // does not need MultiFilter::updateCoefficients()
        f->setOutputAmplitude( amp );

        // get next frequency
        freq = std::max(Float(1), std::min(Float(ao->sampleRate()-1),
                    freqMul * (freqAdd + freq)
                ));
    }
}

void FilterBankAO::setAudioBuffers(uint /*thread*/, uint /*bufferSize*/,
                                   const QList<AUDIO::AudioBuffer *> &/*inputs*/,
                                   const QList<AUDIO::AudioBuffer *> &outputs)
{
    // make a filter for each output
    p_->makeFilters(outputs.size());
}

void FilterBankAO::processAudio(uint , SamplePos pos, uint thread)
{
    const Double time = sampleRateInv() * pos;

    // update filter
    // XXX Note: This is once per dsp-block,
    // not ideal for modulation but much more cpu and cache friendly
    p_->updateFilterCoeffs(time, thread);

    auto filters = &p_->filters[thread];

    // process filter for each channel
    AUDIO::AudioBuffer::process(audioInputs(thread), audioOutputs(thread),
    [filters](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        filters->filters[chan].get()->process(in->readPointer(), out->writePointer(), out->blockSize());
    });
}

} // namespace MO
