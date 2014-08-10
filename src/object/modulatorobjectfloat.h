/** @file modulatorobjectfloat.h

    @brief A float sending modulator object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#ifndef MOSRC_OBJECT_MODULATOROBJECTFLOAT_H
#define MOSRC_OBJECT_MODULATOROBJECTFLOAT_H


#include "modulatorobject.h"

namespace MO {

class ModulatorObjectFloat : public ModulatorObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ModulatorObjectFloat);

    virtual Type type() const { return T_MODULATOR_OBJECT_FLOAT; }

    Double value(Double time, uint thread) const;

signals:

public slots:

private:

};

} // namespace MO

#endif // MOSRC_OBJECT_MODULATOROBJECTFLOAT_H
