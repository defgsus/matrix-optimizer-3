/** @file modulatorfloat.h

    @brief Float modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_ENGINE_MODULATORFLOAT_H
#define MOSRC_ENGINE_MODULATORFLOAT_H

#include "modulator.h"

namespace MO {
namespace IO { class DataStream; }

class Object;

class ModulatorFloat : public Modulator
{
public:

    /** Type of source */
    enum SourceType
    {
        ST_TRACK_FLOAT
    };

    /** Construct a modulator coming form object @p modulatorId
        and belonging to @p parent */
    ModulatorFloat(const QString& modulatorId, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const Q_DECL_OVERRIDE;
    virtual void deserialize(IO::DataStream &) Q_DECL_OVERRIDE;

    // ------------- getter --------------

    Object * parent() const { return parent_; }


private:

    Object * parent_, * modulator_;
};

} // namespace MO

#endif // MOSRC_ENGINE_MODULATORFLOAT_H
