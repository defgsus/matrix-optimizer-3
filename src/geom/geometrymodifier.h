/** @file geometrymodifier.h

    @brief Abstract base of Geometry modifiers

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIER_H
#define MOSRC_GEOM_GEOMETRYMODIFIER_H

#include <QString>

#include "types/float.h"

namespace MO {
namespace IO { class DataStream; }
namespace GEOM {

class Geometry;

class GeometryModifier
{
public:
    GeometryModifier(const QString& className);

    // ------------- getter ------------------

    const QString& className() const { return className_; }

    bool isEnabled() const { return enabled_; }

    // ------------ setter -------------------

    void setEnabled(bool enable) { enabled_ = enable; }

    // ----------- virtual interface ---------

    /** Always call ancestor's method before your derived code. */
    virtual void serialize(IO::DataStream& io) const;
    /** Always call ancestor's method before your derived code. */
    virtual void deserialize(IO::DataStream& io);

    /** Applies the modifications */
    virtual void execute(Geometry * g) = 0;

private:

    QString className_;

    bool enabled_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIER_H
