/** @file parameter.h

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETER_H
#define MOSRC_OBJECT_PARAM_PARAMETER_H

#include <QString>
#include <QList>

#include "object/object_fwd.h"

namespace MO {


class Parameter
{

public:
    Parameter(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    // --------------- getter -------------------

    /** Parent object */
    Object * object() const { return object_; }

    const QString& idName() const { return idName_; }
    const QString& name() const { return name_; }
    /** Returns list of all modulator ids */
    const QList<QString>& modulatorIds() const { return modulatorIds_; }

    bool isEditable() const { return isEditable_; }

    // -------------- setter --------------------

    void setName(const QString& name) { name_ = name; }

    void setEditable(bool enable) { isEditable_ = enable; }

    // ------------ modulators ------------------

    /** Adds an Object to the list of modulators.
        Modulators will be collected by
        collectModulators() in the derived class */
    virtual void addModulator(const QString& idName);

    /** Removes the Object from the list of modulators */
    virtual void removeModulator(const QString& idName);

    virtual void collectModulators() = 0;

    virtual QList<Object*> getModulatingObjects() const = 0;

private:

    Object * object_;

    QString idName_, name_;

    bool isEditable_;

    QList<QString> modulatorIds_;

};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETER_H
