/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include "ModulatorFloatMatrix.h"
#include "io/error.h"
#include "io/DataStream.h"
#include "object/control/TrackFloat.h"
#include "object/control/SequenceFloat.h"
#include "object/control/ModulatorObjectFloat.h"
#include "object/AudioObject.h"
#include "object/interface/ValueFloatMatrixInterface.h"

namespace MO {


ModulatorFloatMatrix::ModulatorFloatMatrix(
        const QString &name, const QString &modulatorId, const QString& outputId,
        Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, ST_FLOAT_MATRIX, parent),
      interface_    (0)
{
}

void ModulatorFloatMatrix::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modfm", 1);
}

void ModulatorFloatMatrix::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modfm", 1);
}

void ModulatorFloatMatrix::copySettingsFrom(const Modulator *other)
{
    const ModulatorFloatMatrix * f = dynamic_cast<const ModulatorFloatMatrix*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorFloatMatrix::copySettingsFrom(" << other << ") "
                   "with non-float-matrix type");
        return;
    }
}

bool ModulatorFloatMatrix::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorFloatMatrix::canBeModulator(NULL) called");

    return
            (dynamic_cast<const ValueFloatMatrixInterface*>(o) != 0)
    ;
}


void ModulatorFloatMatrix::modulatorChanged_()
{
    interface_ = nullptr;

    if (modulator() == nullptr)
        return;

    if (auto iface = dynamic_cast<ValueFloatMatrixInterface*>(modulator()))
    {
        interface_ = iface;
        return;
    }

    MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->idName()
                   << "' to ModulatorFloatMatrix");
}

FloatMatrix ModulatorFloatMatrix::value(const RenderTime& rtime) const
{
    if (!modulator() || !modulator()->active(rtime))
        return FloatMatrix();

    return interface_->valueFloatMatrix(outputChannel(), rtime);
}


} // namespace MO

