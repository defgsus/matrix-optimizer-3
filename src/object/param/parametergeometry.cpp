/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#include "parametergeometry.h"
#include "modulatorgeometry.h"
#include "object/scene.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {

ParameterGeometry::ParameterGeometry(Object * object, const QString& id, const QString& name)
    :   Parameter       (object, id, name)
{
}


void ParameterGeometry::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("pargeo", 1);
}

void ParameterGeometry::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("pargeo", 1);
}

int ParameterGeometry::getModulatorTypes() const
{
    return Object::T_GEOMETRY | Object::TG_REAL_OBJECT;
}

Modulator * ParameterGeometry::getModulator(const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorGeometry(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}


QList<const GEOM::Geometry*> ParameterGeometry::getGeometries(Double time, uint thread) const
{
    QList<const GEOM::Geometry*> list;

    for (auto m : modulators())
        if (auto g = static_cast<ModulatorGeometry*>(m)->value(time, thread))
            list << g;

    if ((int)thread >= lastGeoms_.size())
        lastGeoms_.resize(thread+1);
    lastGeoms_[thread] = list;

    return list;
}


bool ParameterGeometry::hasChanged(Double time, uint thread) const
{
    if ((int)thread >= lastGeoms_.size())
        return true;

    QList<const GEOM::Geometry*> list;
    for (auto m : modulators())
        if (auto g = static_cast<ModulatorGeometry*>(m)->value(time, thread))
            list << g;
    if (list != lastGeoms_[thread])
        return true;

    /** @todo check for geometry change */

    return false;
}

} // namespace MO
