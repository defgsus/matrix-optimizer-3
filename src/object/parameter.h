/** @file parameter.h

    @brief Abstract general purpose parameter for use in Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAMETER_H
#define MOSRC_OBJECT_PARAMETER_H

#include <QSet>

#include "object.h"

namespace MO {


class Parameter : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(Parameter)

    virtual bool isParameter() const { return true; }

    const QString& parameterId() const { return parameterId_; }
    void setParameterId(const QString& id) { parameterId_ = id; }

    /** Adds an Object to the list of modulator.
        Modulators will be collected by
        collectModulators() in the derived class */
    virtual void addModulator(const QString& idName);

    /** Removes the Object from the list of modulators */
    virtual void removeModulator(const QString& idName);

    /** Returns list of all modulator ids */
    virtual const QSet<QString>& getModulators() const { return modulatorIds_; }

signals:

public slots:

private:

    QString parameterId_;

    QSet<QString> modulatorIds_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAMETER_H
