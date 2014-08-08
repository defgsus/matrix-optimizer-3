/** @file audiounit.cpp

    @brief Abstract audio processing object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#include "audiounit.h"
#include "io/datastream.h"
#include "io/error.h"
#include "object/param/parameterselect.h"

namespace MO {


AudioUnit::AudioUnit(QObject *parent) :
    Object(parent)
{
}

void AudioUnit::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("au", 1);
}

void AudioUnit::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("au", 1);
}

void AudioUnit::createParameters()
{
    audioProcessing_ = createSelectParameter(
                        "processmode", tr("processing"),
                        tr("Sets the processing mode"),
                        { "on", "off", "bypass" },
                        { tr("on"), tr("off"), tr("bypass") },
                        { tr("Processing is always on"),
                          tr("Processing is off, no signals are passed through"),
                          tr("The unit does no processing and passes it's input data unchanged") },
                        { PM_ON, PM_OFF, PM_BYPASS },
                        true, false);
}

AudioUnit::ProcessMode AudioUnit::processMode() const
{
    MO_ASSERT(audioProcessing_, "AudioUnit::processMode without parameter");
    return (ProcessMode)audioProcessing_->baseValue();
}

} // namespace MO
