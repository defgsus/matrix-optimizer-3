/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERGEOMETRY_H
#define MOSRC_OBJECT_PARAM_PARAMETERGEOMETRY_H

#include <QVector>
#include <QList>

#include "parameter.h"
#include "types/int.h"
#include "types/float.h"

namespace MO {
namespace GEOM { class Geometry; }

class ParameterGeometry : public Parameter
{
public:

    ParameterGeometry(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("geometry"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_GEOMETRY; }

    QString baseValueString(bool ) const override { return "XXX"; }
    QString valueString(const RenderTime& , bool ) const override { return "XXX"; }

    // ---------------- getter -----------------

    /** Returns true when any of the Geometries have changed since the last call to ... */
    bool hasChanged(const RenderTime& time) const Q_DECL_OVERRIDE;

    /** Returns all Geometries that are connected to this parameter */
    QList<const GEOM::Geometry*> getGeometries(const RenderTime& time) const;

    // ---------------- setter -----------------

    // --------- modulation -----------

    int getModulatorTypes() const Q_DECL_OVERRIDE;

    virtual Modulator * getModulator(const QString &modulatorId, const QString& outputId) Q_DECL_OVERRIDE;

private:

    mutable QVector<QList<const GEOM::Geometry*>> lastGeoms_;
    mutable QVector<QVector<int>> lastHashes_;
};

} // namespace MO

#endif // MOSRC_OBJECT_PARAM_PARAMETERGEOMETRY_H
