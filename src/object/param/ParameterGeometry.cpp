/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#include "ParameterGeometry.h"
#include "ModulatorGeometry.h"
#include "object/Scene.h"
#include "geom/Geometry.h"
#include "io/DataStream.h"
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


QList<const GEOM::Geometry*> ParameterGeometry::getGeometries(const RenderTime & time) const
{
    QList<const GEOM::Geometry*> list;

    for (auto m : modulators())
        if (auto g = static_cast<ModulatorGeometry*>(m)->value(time))
            list << g;

    if ((int)time.thread() >= lastGeoms_.size())
    {
        lastGeoms_.resize(time.thread()+1);
        lastHashes_.resize(time.thread()+1);
    }
    lastGeoms_[time.thread()] = list;
    lastHashes_[time.thread()].resize(list.size());
    for (int i=0; i<list.size(); ++i)
        lastHashes_[time.thread()][i] = list[i]->hash();

    return list;
}


bool ParameterGeometry::hasChanged(const RenderTime& time) const
{
    if ((int)time.thread() >= lastGeoms_.size())
        return true;

    QList<const GEOM::Geometry*> list;
    for (auto m : modulators())
        if (auto g = static_cast<ModulatorGeometry*>(m)->value(time))
            list << g;
    if (list != lastGeoms_[time.thread()])
        return true;

    for (int i=0; i<list.size(); ++i)
        if (list[i]->hash() != lastGeoms_[time.thread()][i]->hash())
            return true;

    return false;
}

} // namespace MO
