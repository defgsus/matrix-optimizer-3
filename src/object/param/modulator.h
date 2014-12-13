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
class Parameter;

/** @brief Internal class to link to modulating objects. */
class Modulator
{
public:

    /** Construct a modulator coming from object @p modulatorId
        and belonging to @p parent / @p param */
    Modulator(const QString& name,
              const QString& modulatorId, const QString& outputId,
              Parameter * parm, Object * parent = 0);
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

    /** Returns some description of source and destination */
    QString nameAutomatic() const;

    /** Returns parent object */
    Object * parent() const { return parent_; }

    /** Returns the set idName for finding the modulating object */
    const QString& modulatorId() const { return modulatorId_; }
    const QString& outputId() const { return outputId_; }

    /** Returns the modulating object */
    Object * modulator() const { return modulator_; }

    Parameter * parameter() const { return param_; }

    /** Returns if the object can be the modulating object */
    virtual bool canBeModulator(const Object *) const = 0;

    /** Returns true if this sticks to an audio output of a module.
        Only valid after setModulator(). */
    bool isAudioToFloatConverter() const;

    // ------------- setter --------------

    /** Changes the modulator id - only used to update new inserted branches */
    void setModulatorId(const QString& id) { modulatorId_ = id; }

    /** Sets the modulating object (from where the modulation comes from).
        Set to NULL to remove the modulator temporarily.
        @note Use canBeModulator() to see if the object fits the requirements. */
    void setModulator(Object * object);

    /** Reimplement this to copy addition parameter settings.
        This does not include the id or something like that. */
    virtual void copySettingsFrom(const Modulator * other) = 0;

protected:

    /** Called when the modulating object has been set. */
    virtual void modulatorChanged_() = 0;

private:

    Object * parent_, * modulator_;

    QString name_, modulatorId_, outputId_;

    Parameter * param_;
};


} // namespace MO



#endif // MOSRC_OBJECT_PARAM_MODULATOR_H
