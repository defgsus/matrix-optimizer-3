/** @file modulatorobject.h

    @brief An abstract object that represents a modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#ifndef MOSRC_OBJECT_MODULATOROBJECT_H
#define MOSRC_OBJECT_MODULATOROBJECT_H


#include "object.h"

namespace MO {

class ModulatorObject : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ModulatorObject);

    bool isModulatorObject() const Q_DECL_OVERRIDE { return true; }

signals:

public slots:

private:

};

} // namespace MO

#endif // MOSRC_OBJECT_MODULATOROBJECT_H
