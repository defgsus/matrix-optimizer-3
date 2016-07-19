/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#include "ModulatorGeometry.h"
//#include "object/control/modulatorobjectfloat.h"
#include "object/Object.h"
#include "object/interface/ValueGeometryInterface.h"
#include "io/DataStream.h"
#include "io/error.h"

namespace MO {


ModulatorGeometry::ModulatorGeometry(
        const QString &name, const QString &modulatorId, const QString& outputId,
        Parameter * p, Object *parent)
    : Modulator     (name, modulatorId, outputId, p, ST_GEOMETRY, parent)
    , geomFace_      (0)
{
}

void ModulatorGeometry::serialize(IO::DataStream & io) const
{
    Modulator::serialize(io);

    io.writeHeader("modgeom", 1);

}

void ModulatorGeometry::deserialize(IO::DataStream & io)
{
    Modulator::deserialize(io);

    io.readHeader("modgeom", 1);

}

void ModulatorGeometry::copySettingsFrom(const Modulator *other)
{
    const ModulatorGeometry * f = dynamic_cast<const ModulatorGeometry*>(other);
    if (!f)
    {
        MO_WARNING("ModulatorGeometry::copySettingsFrom(" << other << ") "
                   "with non-geometry type");
        return;
    }
}

bool ModulatorGeometry::canBeModulator(const Object * o) const
{
    MO_ASSERT(o, "ModulatorGeometry::canBeModulator(NULL) called");

    return dynamic_cast<const ValueGeometryInterface*>(o) != 0;
}


void ModulatorGeometry::modulatorChanged_()
{
    geomFace_ = 0;

    if (modulator() == 0)
        return;

    if (auto tex = dynamic_cast<ValueGeometryInterface*>(modulator()))
        geomFace_ = tex;

    else
    {
        MO_ASSERT(false, "illegal assignment of modulator '" << modulator()->idName()
                       << "' to ModulatorGeometry");
    }
}

const GEOM::Geometry * ModulatorGeometry::value(const RenderTime& time) const
{
    if (!modulator() || !modulator()->active(time))
        return 0;

    if (geomFace_)
        return geomFace_->valueGeometry(outputChannel(), time);

    return 0;
}


} // namespace MO
