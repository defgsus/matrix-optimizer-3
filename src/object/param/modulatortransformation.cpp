/** @file modulatortransformation.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#include "modulatortransformation.h"
#include "object/interface/valuetransformationinterface.h"
#include "object/object.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {


ModulatorTransformation::ModulatorTransformation(
            const QString &name, const QString &modulatorId, const QString& outputId,
            Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, parent)
    , channel_      (0)
    , interface_    (0)
{
}

void ModulatorTransformation::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modtr", 1);
}

void ModulatorTransformation::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modtr", 1);
}

void ModulatorTransformation::copySettingsFrom(const Modulator *other)
{
    const ModulatorTransformation * f = dynamic_cast<const ModulatorTransformation*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorTransformation::copySettingsFrom(" << other << ") "
                   "with non-float type");
        return;
    }

    //amplitude_ = f->amplitude();
    //timeOffset_ = f->timeOffset();
}

bool ModulatorTransformation::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorTransformation::canBeModulator(NULL) called");

    return (dynamic_cast<const ValueTransformationInterface*>(o) != 0)
    ;
}


void ModulatorTransformation::modulatorChanged_()
{
   if (auto iface = dynamic_cast<ValueTransformationInterface*>(modulator()))
    {
        interface_ = iface;
    }
    else
    {
        interface_ = 0;
        MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->idName()
                       << "' to ModulatorTransformation");
    }
}

Mat4 ModulatorTransformation::value(Double time, uint thread) const
{
    if (!modulator() || !modulator()->active(time, thread))
        return Mat4(1.);

    return interface_->valueTransformation(time, thread);
}


} // namespace MO
