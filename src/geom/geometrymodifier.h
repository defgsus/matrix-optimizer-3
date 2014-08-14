/** @file geometrymodifier.h

    @brief Abstract base of Geometry modifiers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIER_H
#define MOSRC_GEOM_GEOMETRYMODIFIER_H

#include <QString>
#include <QObject> // for tr() in derived classes

#include "types/float.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

/** Call this somewhere in your modifier .cpp file to register the modifier */
#define MO_REGISTER_GEOMETRYMODIFIER(Class__) \
    namespace { bool registered_##Class__ = ::MO::GEOM::registerGeometryModifier_(new Class__); }


class Geometry;

class GeometryModifier
{
public:
    GeometryModifier(const QString& className, const QString& guiName);
    virtual ~GeometryModifier() { }

    // ------------- getter ------------------

    const QString& className() const { return className_; }
    const QString& guiName() const { return guiName_; }

    bool isEnabled() const { return enabled_; }

    // ------------ setter -------------------

    void setEnabled(bool enable) { enabled_ = enable; }

    // ----------- virtual interface ---------

    /** Always call ancestor's method before your derived code. */
    virtual void serialize(IO::DataStream& io) const;
    /** Always call ancestor's method before your derived code. */
    virtual void deserialize(IO::DataStream& io);

    /** Returns a new class of the modifier */
    virtual GeometryModifier * cloneClass() const = 0;

    /** Applies the modifications */
    virtual void execute(Geometry * g) = 0;

private:

    QString className_, guiName_;

    bool enabled_;
};

bool registerGeometryModifier_(GeometryModifier *);

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIER_H
