/** @file filterbankunit.cpp

    @brief Filterbank AudioUnit object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/11/2014</p>
*/

#include "filterbankunit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/param/parameters.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "audio/tool/multifilter.h"

namespace MO {

MO_REGISTER_OBJECT(FilterBankUnit)

FilterBankUnit::FilterBankUnit(QObject *parent) :
    AudioUnit(1, -1, false, parent)
{
    setName("FilterBankUnit");
}

FilterBankUnit::~FilterBankUnit()
{
    for (auto f : filter_)
        delete f;
}

void FilterBankUnit::serialize(IO::DataStream & io) const
{
    AudioUnit::serialize(io);
    io.writeHeader("aufilterbank", 1);
}

void FilterBankUnit::deserialize(IO::DataStream & io)
{
    AudioUnit::deserialize(io);
    io.readHeader("aufilterbank", 1);
}

void FilterBankUnit::createParameters()
{
    AudioUnit::createParameters();

    params()->beginParameterGroup("channels", tr("channels"));

        numOut_ = params()->createIntParameter("numout", tr("number outputs"),
                                     tr("The number of individual filters in the filter bank"),
                                     numChannelsOut(),
                                     1, MO_MAX_AUDIO_CHANNELS,
                                     1, true, false);

    params()->endParameterGroup();

    params()->beginParameterGroup("filtset", tr("filter settings"));

        type_ = params()->createSelectParameter("type", tr("filter type"),
                                      tr("Selectes the type of filter"),
                                      AUDIO::MultiFilter::filterTypeIds,
                                      AUDIO::MultiFilter::filterTypeNames,
                                      AUDIO::MultiFilter::filterTypeStatusTips,
                                      AUDIO::MultiFilter::filterTypeEnums,
                                      AUDIO::MultiFilter::T_FIRST_ORDER_LOW, true, false);

        filterOrder_ = params()->createIntParameter("forder", tr("filter order"),
                                     tr("The order (sharpness) of the filter for the 'nth order' types"),
                                     1,
                                     1, 20,
                                     1, true, false);

        baseFreq_ = params()->createFloatParameter("freq", tr("base frequency"),
                                     tr("Controls the filter base frequency (of the first output) in Hertz"),
                                     200., 10.);
        baseFreq_->setRange(0.0001, 100000.0);

        addFreq_ = params()->createFloatParameter("freqadd", tr("add frequency"),
                                     tr("Frequency to be added after each stage"),
                                     0., 10.);
        addFreq_->setRange(-100000.0, 100000.0);

        mulFreq_ = params()->createFloatParameter("freqmul", tr("multiply frequency"),
                                     tr("Factor to multiply the frequency after each stage"),
                                     2., 0.05);
        mulFreq_->setRange(-1000.0, 1000.0);

        reso_ = params()->createFloatParameter("reso", tr("resonance"),
                                     tr("Controls the filter resonance - how much of the filter is fed-back to itself"),
                                     0.0, 0.01);
        reso_->setRange(0.0, 1.0);

    params()->endParameterGroup();
}

void FilterBankUnit::onParameterChanged(Parameter *p)
{
    AudioUnit::onParameterChanged(p);

    if (p == numOut_)
        requestNumChannelsOut_(numOut_->baseValue());
}

void FilterBankUnit::onParametersLoaded()
{
    AudioUnit::onParametersLoaded();

    requestNumChannelsOut_(numOut_->baseValue());
}

void FilterBankUnit::channelsChanged()
{
    AudioUnit::channelsChanged();

    createFilters_();
}

void FilterBankUnit::setNumberThreads(uint num)
{
    AudioUnit::setNumberThreads(num);

    createFilters_();
}

void FilterBankUnit::createFilters_()
{
    // clear old
    for (auto f : filter_)
        delete f;
    filter_.clear();

    // create new
    const uint num = numChannelsOut() * numberThreads();
    for (uint i = 0; i<num; ++i)
        filter_.push_back( new AUDIO::MultiFilter() );
}

void FilterBankUnit::processAudioBlock(const F32 *input, Double time, uint thread)
{
    MO_ASSERT(thread < filter_.size(), "thread " << thread << " out of range for "
              "FilterBankUnit with " << filter_.size() << " threads");

    const F32 basefreq = baseFreq_->value(time, thread),
              reso = reso_->value(time, thread),
              fadd = addFreq_->value(time, thread),
              fmul = mulFreq_->value(time, thread);
    const uint order = filterOrder_->value(time, thread);

    const auto type = AUDIO::MultiFilter::FilterType(type_->baseValue());

    const uint bsize = bufferSize(thread);

    F32 freq = basefreq;

    for (uint i=0; i<numChannelsOut(); ++i)
    {
        AUDIO::MultiFilter * filter = filter_[i * numberThreads() + thread];

        // adjust filter settings
        if (freq != filter->frequency()
                || reso != filter->resonance()
                || sampleRate() != filter->sampleRate()
                || type != filter->type()
                || order != filter->order())
        {
            filter->setFrequency(freq);
            filter->setResonance(reso);
            filter->setSampleRate(sampleRate());
            filter->setType(type);
            filter->setOrder(order);
            filter->updateCoefficients();
        }

        // move frequency forward
        freq = std::max((F32)0.0001, (freq + fadd) * fmul);

        // one input, many outputs
        filter->process(&input[0], &outputBuffer(thread)[i*bsize], bsize);
    }
}


} // namespace MO
