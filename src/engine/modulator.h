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

    /** Stores the settings to the stream.
        Always call the ancestor function in your derived function! */
    virtual void serialize(IO::DataStream &) const;

    /** Restores the settings from the stream.
        Always call the ancestor function in your derived function! */
    virtual void deserialize(IO::DataStream &);

    // ------------- getter --------------

    /** Returns parent object */
    Object * parent() const { return parent_; }

    /** Returns the set idName for finding the modulating object */
    const QString& modulatorId() const { return modulatorId_; }

    /** Returns the modulating object */
    Object * modulator() const { return modulator_; }

    // ------------- setter --------------

    void setModulator(Object * object) { modulator_ = object; modulatorChanged_(); }

protected:

    /** Called when the modulating object has been set. */
    virtual void modulatorChanged_() = 0;

private:

    Object * parent_, * modulator_;

    QString modulatorId_;
};

} // namespace MO

#endif // MOSRC_ENGINE_MODULATOR_H
