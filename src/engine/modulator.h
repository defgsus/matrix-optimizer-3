/** @file modulator.h

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_ENGINE_MODULATOR_H
#define MOSRC_ENGINE_MODULATOR_H

#include <QString>

namespace MO {
namespace IO { class DataStream; }

class Object;

class Modulator
{
public:

    /** Construct a modulator coming form object @p modulatorId
        and belonging to @p parent */
    Modulator(const QString& modulatorId, Object * parent = 0);

    // --------------- io ----------------

    virtual void serialize(IO::DataStream &) const = 0;
    virtual void deserialize(IO::DataStream &) = 0;

    // ------------- getter --------------

    /** Returns parent object */
    Object * parent() const { return parent_; }

    /** Returns the idName of the modulating object */
    const QString& modulatorId() const { return modulatorId_; }

private:

    Object * parent_;

    QString modulatorId_;
};

} // namespace MO

#endif // MOSRC_ENGINE_MODULATOR_H
