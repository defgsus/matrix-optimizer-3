/** @file modulator.h

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATOR_H
#define MOSRC_OBJECT_PARAM_MODULATOR_H

#include <QString>

namespace MO {
namespace IO { class DataStream; }

class Object;

/** @brief Internal class to link to modulating objects. */
class Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent */
    Modulator(const QString& name, const QString& modulatorId, Object * parent = 0);
    virtual ~Modulator() { }

    // --------------- io ----------------

    /** Stores the settings to the stream.
        Always call the ancestor function in your derived function! */
    virtual void serialize(IO::DataStream &) const;

    /** Restores the settings from the stream.
        Always call the ancestor function in your derived function! */
    virtual void deserialize(IO::DataStream &);

    // ------------- getter --------------

    /** Returns the name of the modulator
        (typically the name of the Parameter that is modulated) */
    const QString& name() const { return name_; }

    /** Returns parent object */
    Object * parent() const { return parent_; }

    /** Returns the set idName for finding the modulating object */
    const QString& modulatorId() const { return modulatorId_; }

    /** Returns the modulating object */
    Object * modulator() const { return modulator_; }

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const = 0;

    // ------------- setter --------------

    /** Sets the modulating object (from where the modulation comes from).
        Set to NULL to remove the modulator temporarily.
        @note Use canBeModulator() to see if the object fits the requirements. */
    void setModulator(Object * object);

protected:

    /** Called when the modulating object has been set. */
    virtual void modulatorChanged_() = 0;

private:

    Object * parent_, * modulator_;

    QString name_, modulatorId_;
};


} // namespace MO



#endif // MOSRC_OBJECT_PARAM_MODULATOR_H
