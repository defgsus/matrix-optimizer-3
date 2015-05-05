/** @file geometrymodifierenum.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.05.2015</p>
*/

#ifndef MOSRC_GEOM_GEOMETRYMODIFIERENUM_H
#define MOSRC_GEOM_GEOMETRYMODIFIERENUM_H

#include "geometrymodifier.h"

namespace MO {
namespace GEOM {


class GeometryModifierEnum : public GeometryModifier
{
public:
    MO_GEOMETRYMODIFIER_CONSTRUCTOR(GeometryModifierEnum)

    // ------------- getter ------------------

    bool getDoIndex() const { return doIndex_; }
    const QString& getIndexName() const { return indexName_; }

    // ------------ setter -------------------

    void setDoIndex(bool e) { doIndex_ = e; }
    void setIndexName(const QString& n) { indexName_ = n; }

private:

    bool doIndex_;
    QString indexName_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_GEOMETRYMODIFIERENUM_H
