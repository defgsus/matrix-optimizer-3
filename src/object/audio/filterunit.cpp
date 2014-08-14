/** @file filterunit.cpp

    @brief FilterUnit that filters input in specific modes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "filterunit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/param/parameterselect.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfloat.h"
#include "audio/tool/multifilter.h"

namespace MO {

MO_REGISTER_OBJECT(FilterUnit)

FilterUnit::FilterUnit(QObject *parent) :
    AudioUnit(-1, -1, true, parent)
{
    setName("FilterUnit");
}

FilterUnit::~FilterUnit()
{
    for (auto f : filter_)
        delete f;
}

void FilterUnit::serialize(IO::DataStream & io) const
{
    AudioUnit::serialize(io);
    io.writeHeader("aufilter", 1);
}

void FilterUnit::deserialize(IO::DataStream & io)
{
    AudioUnit::deserialize(io);
    io.readHeader("aufilter", 1);
}

void FilterUnit::createParameters()
{
    AudioUnit::createParameters();

    beginParameterGroup("filtset", tr("filter settings"));

    type_ = createSelectParameter("type", tr("filter type"),
                                  tr("Selectes the type of filter"),
                                  AUDIO::MultiFilter::filterTypeIds,
                                  AUDIO::MultiFilter::filterTypeNames,
                                  AUDIO::MultiFilter::filterTypeStatusTips,
                                  AUDIO::MultiFilter::filterTypeEnums,
                                  AUDIO::MultiFilter::T_FIRST_ORDER_LOW, true, false);

    filterOrder_ = createIntParameter("forder", tr("filter order"),
                                 tr("The order (sharpness) of the filter for the 'nth order' types"),
                                 1,
                                 1, 20,
                                 1, true, false);

    freq_ = createFloatParameter("freq", tr("frequency"),
                                 tr("Controls the filter frequency in Hertz"),
                                 1000., 10.);
    freq_->setRange(0.0001, 100000.0);

    reso_ = createFloatParameter("reso", tr("resonance"),
                                 tr("Controls the filter resonance - how much of the filter is fed-back to itself"),
                                 0.0, 0.01);
    reso_->setRange(0.0, 1.0);

    endParameterGroup();

}

void FilterUnit::channelsChanged()
{
    AudioUnit::channelsChanged();

    createFilters_();
}

void FilterUnit::setNumberThreads(uint num)
{
    AudioUnit::setNumberThreads(num);
    createFilters_();
}

void FilterUnit::createFilters_()
{
    // clear old
    for (auto f : filter_)
        delete f;
    filter_.clear();

    // create new
    const uint num = numChannelsIn() * numberThreads();
    for (uint i = 0; i<num; ++i)
        filter_.push_back( new AUDIO::MultiFilter() );
}

void FilterUnit::processAudioBlock(const F32 *input, Double time, uint thread)
{
    MO_ASSERT(thread < filter_.size(), "thread " << thread << " out of range for "
              "FilterUnit with " << filter_.size() << " threads");

    const F32 freq = freq_->value(time, thread),
              reso = reso_->value(time, thread);
    const uint order = filterOrder_->value(time, thread);

    const auto type = AUDIO::MultiFilter::FilterType(type_->baseValue());

    const uint bsize = bufferSize(thread);

    for (uint i=0; i<numChannelsIn(); ++i)
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
            filter->updateCoefficients();
        }

        filter->process(&input[i*bsize], &outputBuffer(thread)[i*bsize], bsize);
    }
}


} // namespace MO
