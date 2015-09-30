/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_VALUEGEOMETRYINTERFACE_H
#define MOSRC_OBJECT_INTERFACE_VALUEGEOMETRYINTERFACE_H

#include "types/float.h"

namespace MO {
namespace GEOM { class Geometry; }

/** A geometry output. */
class ValueGeometryInterface
{
public:
    virtual ~ValueGeometryInterface() { }

    /** Return a GEOM::Geometry or NULL. */
    virtual const GEOM::Geometry * valueGeometry(uint channel, Double time, uint thread) const = 0;

};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_VALUEGEOMETRYINTERFACE_H

