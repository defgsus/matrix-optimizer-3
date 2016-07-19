/** @file filterbankao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/
#include <QDebug>
#include <memory>

#include "FilterBankAO.h"
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

MO_REGISTER_OBJECT(FilterBankAO)

class FilterBankAO::Private
{
    public:

    Private(FilterBankAO * ao)
        : ao            (ao)
        , readIoVer     (2)
    { }
    FilterBankAO * ao;

    void makeFilters(uint num);
    void updateFilterCoeffs(const RenderTime & time);

    int readIoVer;
    ParameterFloat
        * paramFreqStart,
        * paramFreqEnd,
        * paramFreqAdd,
        * paramFreqMul,
        * paramFreqExp,
        * paramReso,
        * paramAmp,
        * paramAmpHighBoost;
        //* paramReset;
    ParameterInt
        * paramOrder;
    ParameterSelect
        * paramType,
        * paramFreqScale;
    ParameterCallback
        * paramReset;

    // filters for all channels
    struct Filters
    {
        std::vector<std::shared_ptr<AUDIO::MultiFilter>> filters;
    };

    // per thread
    std::vector<Filters> filters;
    bool doReset;
};

FilterBankAO::FilterBankAO()
    : AudioObject   (),
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

    io.writeHeader("aofbank", 2);
    // v2 adds default paramFreqScale
}

void FilterBankAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    p_->readIoVer = io.readHeader("aofbank", 2);
}

void FilterBankAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("filter", tr("Filter"));
    initParameterGroupExpanded("filter");

        p_->paramReset = params()->createCallbackParameter(
                "reset", tr("reset"),
                tr("Resets the filter (useful in case of internal overflow"),
                [=](){ p_->doReset = true; });

        p_->paramType = params()->createSelectParameter(
                    "_filter_type", tr("filter type"),
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

        p_->paramFreqScale = params()->createSelectParameter(
                    "_freq_scale", tr("frequence scale"),
                  tr("Selectes of frequency values for each band"),
        { "addmul", "startend" },
        { tr("add/mul"), tr("start to end") },
        { tr("Constant addition and multiplication at each band"),
          tr("Linear/exponentially spaced distances between start and end frequency") },
        { 0, 1 },
        1, true, false);
        if (p_->readIoVer < 2)
            p_->paramFreqScale->setValue(0);


        p_->paramFreqStart = params()->createFloatParameter(
                    "_filter_freq_start", tr("start frequency"),
                   tr("The first band's frequency of the filter in Hertz"),
                                               100.0, 10.0);
        p_->paramFreqEnd = params()->createFloatParameter(
                    "_filter_freq_end", tr("end frequency"),
                   tr("The last band's frequency of the filter in Hertz"),
                                               10000.0, 10.0);
        p_->paramFreqAdd = params()->createFloatParameter(
                    "_filter_freq_add", tr("+ frequency"),
                    tr("The frequency to add to the start frequency for "
                       "each channel in Hertz"),
                    0.0, 10.0);
        p_->paramFreqMul = params()->createFloatParameter(
                    "_filter_freq_mul", tr("* frequency"),
                    tr("The factor to multiply the current frequency "
                       "for the next channel"),
                                                   1.0, 0.0625);
        p_->paramFreqExp = params()->createFloatParameter(
                    "_filter_freq_exp", tr("scale exponent"),
                   tr("The exponent on the normalized frequency transition "
                      "between start and end frequency"),
                                               2.0, 0.025);
        p_->paramFreqExp->setMinValue(0.0001);

        p_->paramReso = params()->createFloatParameter(
                    "_filter_reso", tr("resonance"),
                    tr("The filter steepness [0,1]"),
                    0.0, 0.02);
        p_->paramAmp = params()->createFloatParameter(
                    "_filter_amp", tr("amplitude"),
                    tr("The output amplitude of the filter"),
                    1.0, 0.05);
        p_->paramAmpHighBoost = params()->createFloatParameter(
                    "_filter_amp_highboost", tr("amplitude high-boost"),
                    tr("Increase of amplitude per frequence band"),
                    0.0, 0.05);

    params()->endParameterGroup();
}

void FilterBankAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    auto type = (AUDIO::MultiFilter::FilterType)p_->paramType->baseValue();
    bool isStartEnd = p_->paramFreqScale->baseValue() == 1;

    p_->paramOrder->setVisible(
                AUDIO::MultiFilter::supportsOrder(type));
    p_->paramReso->setVisible(
                AUDIO::MultiFilter::supportsResonance(type));
    p_->paramFreqAdd->setVisible(!isStartEnd);
    p_->paramFreqMul->setVisible(!isStartEnd);
    p_->paramFreqEnd->setVisible(isStartEnd);
    p_->paramFreqExp->setVisible(isStartEnd);
}

void FilterBankAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);
}

void FilterBankAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

}

void FilterBankAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

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

void FilterBankAO::Private::updateFilterCoeffs(const RenderTime& time)
{
    paramReset->fireIfInput(time);
    if (doReset)
    {
        doReset = false;
        for (auto & fp : filters[time.thread()].filters)
            fp.get()->reset();
    }

    F32     freq = paramFreqStart->value(time),
            freqStart = freq,
            freqEnd = paramFreqEnd->value(time),
            freqAdd = paramFreqAdd->value(time),
            freqMul = paramFreqMul->value(time),
            res = paramReso->value(time),
            amp = paramAmp->value(time),
            highBoost = paramAmpHighBoost->value(time),
            invK = filters[time.thread()].filters.size() > 1 ?
                1.f / F32(filters[time.thread()].filters.size() - 1) : 1.f,
            expK = paramFreqExp->value(time);
    int     order = paramOrder->value(time);
    auto    type = (AUDIO::MultiFilter::FilterType)paramType->baseValue();
    int     scaleMode = paramFreqScale->baseValue();

    // each filter per channel
    int k = 0;
    for (auto & fp : filters[time.thread()].filters)
    {
        auto f = fp.get();

        if (scaleMode == 1)
        {
            freq = freqStart + (freqEnd - freqStart)
                    * std::pow(k * invK, expK);
            //std::cout << k << " " << freq << " " << (1./invK) << std::endl;
        }

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
        f->setOutputAmplitude( amp + highBoost * k );
        ++k;

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

void FilterBankAO::processAudio(const RenderTime& time)
{
    // update filter
    // XXX Note: This is once per dsp-block,
    // not ideal for modulation but much more cpu and cache friendly
    p_->updateFilterCoeffs(time);

    auto filters = &p_->filters[time.thread()];

    // process filter for each channel
    AUDIO::AudioBuffer::process(audioInputs(time.thread()),
                                audioOutputs(time.thread()),
    [filters](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        filters->filters[chan].get()->process(
                    in->readPointer(), out->writePointer(), out->blockSize());
    });
}

} // namespace MO
