/** @file bandfilterbankao.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.12.2014</p>
*/

#include <memory>

#include "bandfilterbankao.h"
#include "audio/tool/audiobuffer.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/constants.h"
#include "audio/tool/fixedfilter.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(BandFilterBankAO)

class BandFilterBankAO::Private
{
    public:

    // important for number of parameters
    static const uint maxChannels = 16;

    Private(BandFilterBankAO * ao) : ao(ao) { }
    BandFilterBankAO * ao;

    AUDIO::FixedFilter::FilterType filterType() const
        { return (AUDIO::FixedFilter::FilterType)paramType->baseValue(); }

    void makeFilters(uint num);
    void updateFilterCoeffs(const RenderTime& time);

    std::vector<ParameterFloat*>
        paramFreqStart,
        paramFreqWidth,
        paramAmp;
    ParameterFloat
        * paramMetaAmp;
    ParameterInt
        * paramOrder;
    ParameterSelect
        * paramType;

    // filters for all channels
    struct Filters
    {
        std::vector<std::shared_ptr<AUDIO::FixedFilter>> filters;
    };

    // per thread
    std::vector<Filters> filters;
};

BandFilterBankAO::BandFilterBankAO(QObject *parent)
    : AudioObject   (parent),
      p_            (new Private(this))
{
    setName("BandFilterBank");
    setNumberChannelsAdjustable(true);
}

BandFilterBankAO::~BandFilterBankAO()
{
    delete p_;
}

void BandFilterBankAO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);

    io.writeHeader("aofbank", 1);
}

void BandFilterBankAO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);

    io.readHeader("aofbank", 1);
}

void BandFilterBankAO::createParameters()
{
    AudioObject::createParameters();

    params()->beginParameterGroup("filter", tr("Filter"));
    initParameterGroupExpanded("filter");

        p_->paramType = params()->createSelectParameter("_filter_type", tr("filter type"),
                                                  tr("Selectes the type of filter"),
                                                  { "bessel", "cheby", "butterw" },
                                                  { tr("Bessel"), tr("Chebychev"), tr("Butterworth") },
                                                  { tr("Bessel filter type"),
                                                    tr("Chebychev filter type"),
                                                    tr("Butterworth filter type") },
                                                  { AUDIO::FixedFilter::FT_BESSEL, AUDIO::FixedFilter::FT_CHEBYCHEV, AUDIO::FixedFilter::FT_BUTTERWORTH },
                                                  AUDIO::FixedFilter::FT_BESSEL,
                                                  true, false);

        p_->paramOrder = params()->createIntParameter("_filter_order", tr("order"),
                                                   tr("The order (quality/exactness) of the filter."),
                                                   2,
                                                   1, 10,
                                                   1, true, false);

        p_->paramMetaAmp = params()->createFloatParameter("_filter_amp",
                                                            tr("main amplitude"),
                                                            tr("The output amplitude of all filter bands - main multiplier"),
                                                            1.0, 0.05);

        // parameters per band

        p_->paramFreqStart.resize(p_->maxChannels);
        p_->paramFreqWidth.resize(p_->maxChannels);
        p_->paramAmp.resize(p_->maxChannels);

        for (uint i = 0; i < p_->maxChannels; ++i)
        {
            p_->paramFreqStart[i] = params()->createFloatParameter(QString("_filter_freq_start_%1").arg(i + 1),
                                                                tr("%1: frequency").arg(i + 1),
                                                                tr("The frequency of the filter band in Hertz"),
                                                                i * 100.0, 10.0);
            p_->paramFreqStart[i]->setMinValue(0.0);

            p_->paramFreqWidth[i] = params()->createFloatParameter(QString("_filter_freq_width_%1").arg(i + 1),
                                                                tr("%1: width").arg(i + 1),
                                                                tr("The width of the filter band in Hertz - "
                                                                   "the actual band range is center + [-width/2, width/2]"),
                                                                50.0, 1.0);
            p_->paramFreqWidth[i]->setMinValue(0.0);

            p_->paramAmp[i] = params()->createFloatParameter(QString("_filter_amp_%1").arg(i+1),
                                                                tr("%1: amplitude").arg(i+1),
                                                                tr("The output amplitude of the filter band"),
                                                                1.0, 0.05);
        }

        //p_->paramReset = params()->createGateParameter

    params()->endParameterGroup();
}

void BandFilterBankAO::updateParameterVisibility()
{
    AudioObject::updateParameterVisibility();

    for (uint i=0; i<p_->maxChannels; ++i)
    {
        const bool v = i < numAudioOutputs();
        p_->paramFreqStart[i]->setVisible(v);
        p_->paramFreqWidth[i]->setVisible(v);
        p_->paramAmp[i]->setVisible(v);
    }
}

void BandFilterBankAO::onParameterChanged(Parameter * p)
{
    AudioObject::onParameterChanged(p);

}

void BandFilterBankAO::onParametersLoaded()
{
    AudioObject::onParametersLoaded();

}

void BandFilterBankAO::setNumberThreads(uint count)
{
    AudioObject::setNumberThreads(count);

}

void BandFilterBankAO::Private::makeFilters(uint num)
{
    // filters for each thread
    filters.resize(ao->numberThreads());
    for (auto & fs : filters)
    {
        fs.filters.clear();
        // filter for each channel
        for (uint i=0; i<num; ++i)
        {
            auto f = new AUDIO::FixedFilter();
            f->setBandType(AUDIO::FixedFilter::BT_BANDPASS);

            auto sp = std::shared_ptr<AUDIO::FixedFilter>(f);
            fs.filters.push_back(sp);
        }
    }
}

void BandFilterBankAO::Private::updateFilterCoeffs(const RenderTime & time)
{
    int     order = paramOrder->value(time);

    // each filter per channel
    for (uint k = 0; k < filters[time.thread()].filters.size(); ++k)
    {
        AUDIO::FixedFilter * f = filters[time.thread()].filters[k].get();

        Double  freq = std::max(1., paramFreqStart[k]->value(time)),
                width = std::max(1., paramFreqWidth[k]->value(time));

        // test for changes (to avoid updateCoefficients())
        if (
               f->frequency() != freq
            || f->bandpassSize() != width
            || (int)f->order() != order
            || f->sampleRate() != ao->sampleRate()
            || f->type() != filterType())
        {
            f->setFrequency(freq);
            f->setBandpassSize(width);
            f->setSampleRate(ao->sampleRate());
            f->setType(filterType());
            f->setOrder(order);
            f->updateCoefficients();
        }
    }
}

void BandFilterBankAO::setAudioBuffers(uint /*thread*/, uint /*bufferSize*/,
                                   const QList<AUDIO::AudioBuffer *> &/*inputs*/,
                                   const QList<AUDIO::AudioBuffer *> &outputs)
{
    // make a filter for each output
    p_->makeFilters(std::min(outputs.size(), (int)p_->maxChannels));
}

void BandFilterBankAO::processAudio(const RenderTime& time)
{
    const Double
            metaamp = p_->paramMetaAmp->value(time);

    // update filter
    // XXX Note: This is once per dsp-block,
    p_->updateFilterCoeffs(time);

    auto filters = &p_->filters[time.thread()];

    // process filter for each channel
    AUDIO::AudioBuffer::process(audioInputs(time.thread()), audioOutputs(time.thread()),
    [=](uint chan, const AUDIO::AudioBuffer * in, AUDIO::AudioBuffer * out)
    {
        if (chan < p_->maxChannels)
            filters->filters[chan].get()->process(
                        in->readPointer(), out->writePointer(), out->blockSize(),
                        metaamp * p_->paramAmp[chan]->value(time));
    });
}

} // namespace MO
