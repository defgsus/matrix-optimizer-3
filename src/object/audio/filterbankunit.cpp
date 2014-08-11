/** @file filterbankunit.cpp

    @brief Filterbank AudioUnit object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/11/2014</p>
*/

#include "filterbankunit.h"
#include "io/datastream.h"
#include "io/error.h"
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

    numOut_ = createIntParameter("numout", tr("number outputs"),
                                   tr("The number of individual filters in the filter bank"),
                                   1, 1, true, false);

    type_ = createSelectParameter("type", tr("filter type"),
                                  tr("Selectes the type of filter"),
                                  AUDIO::MultiFilter::filterTypeIds,
                                  AUDIO::MultiFilter::filterTypeNames,
                                  AUDIO::MultiFilter::filterTypeStatusTips,
                                  AUDIO::MultiFilter::filterTypeEnums,
                                  AUDIO::MultiFilter::T_FIRST_ORDER_LOW, true, false);

    baseFreq_ = createFloatParameter("freq", tr("frequency"),
                                 tr("Controls the filter frequency in Hertz"),
                                 1000., 10.);
    baseFreq_->setRange(0.0001, 100000.0);

    reso_ = createFloatParameter("reso", tr("resonance"),
                                 tr("Controls the filter resonance - how much of the filter is fed-back to itself"),
                                 0.0, 0.01);
    reso_->setRange(0.0, 1.0);

}

void FilterBankUnit::channelsChanged(uint thread)
{
    AudioUnit::channelsChanged(thread);

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

    const F32 freq = baseFreq_->value(time, thread),
              reso = reso_->value(time, thread);
    const auto type = AUDIO::MultiFilter::FilterType(type_->baseValue());

    const uint bsize = bufferSize(thread);

    for (uint i=0; i<numChannelsOut(); ++i)
    {
        AUDIO::MultiFilter * filter = filter_[i * numberThreads() + thread];

        // adjust filter settings
        if (freq != filter->frequency()
                || reso != filter->resonance()
                || sampleRate() != filter->sampleRate()
                || type != filter->type())
        {
            filter->setFrequency(freq);
            filter->setResonance(reso);
            filter->setSampleRate(sampleRate());
            filter->setType(type);
            filter->updateCoefficients();
        }

        // one input, many outputs
        filter->process(&input[0], &outputBuffer(thread)[i*bsize], bsize);
    }
}


} // namespace MO
