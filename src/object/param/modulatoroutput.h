/** @file modulatoroutput.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_MODULATOROUTPUT_H
#define MOSRC_OBJECT_PARAM_MODULATOROUTPUT_H

#include "types/int.h"
#include "types/float.h"

#include <QString>

namespace MO {

class Object;


// **** XXX NOT USEd YET ****


class ModulatorOutput
{
    // to set p_channel_;
    friend class Object;
public:
    /** Constructs an output for an Object.
        The id is object-unique and PERSISTENT.
        Changing it would require to change the id in Modulator. */
    ModulatorOutput(Object * o, const QString& id);
    virtual ~ModulatorOutput() { }

    // ------------------ getter -----------------

    /** Returns parent object */
    Object * object() const { return p_object_; }

    /** Returns the PERSISTENT id of the output */
    const QString& id() const { return p_id_; }

    /** Returns the channel of the output */
    uint channel() const { return p_channel_; }

private:

    Object * p_object_;
    QString p_id_;
    uint p_channel_;
};



template <typename T>
class ModulatorOutputStatic : public ModulatorOutput
{
public:

    /** Constructs a float output for an Object.
        The id is object-unique and PERSISTENT.
        Changing it would require to change the id in Modulator. */
    ModulatorOutputStatic(Object * o, const QString& id, T initValue)
        : ModulatorOutput   (o, id),
          p_value_          (initValue)
    { }

    // ----- getter -------

    T value() const { return p_value_; }

    // ----- setter -------

    void setValue(const T value) { p_value_ = value; }

private:

    T p_value_;
};

// default type for floating point
typedef ModulatorOutputStatic<Double> ModulatorOutputStaticFloat;





} // namespace MO

#endif // MOSRC_OBJECT_PARAM_MODULATOROUTPUT_H


