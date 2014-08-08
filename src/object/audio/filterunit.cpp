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

namespace MO {


FilterUnit::FilterUnit(QObject *parent) :
    AudioUnit(0, 0, true, parent)
{
    setName("FilterUnit");
}

void FilterUnit::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("aufilter", 1);
}

void FilterUnit::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("aufilter", 1);
}

void FilterUnit::createParameters()
{
    /*
    processModeParameter_ = createSelectParameter(
                        "processmode", tr("processing"),
                        tr("Sets the processing mode"),
                        { "on", "off", "bypass" },
                        { tr("on"), tr("off"), tr("bypass") },
                        { tr("Processing is always on"),
                          tr("Processing is off, no signals are passed through"),
                          tr("The unit does no processing and passes it's input data unchanged") },
                        { PM_ON, PM_OFF, PM_BYPASS },
                        true, false);
    */
}

void FilterUnit::processAudioBlock(const F32 *input, F32 *output, Double time, uint thread)
{

}


} // namespace MO
