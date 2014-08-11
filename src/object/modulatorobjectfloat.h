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

    virtual Type type() const Q_DECL_OVERRIDE { return T_MODULATOR_OBJECT_FLOAT; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    Double value(Double time, uint thread) const;

    /** Sets the base-value.
        This is an offset independent of the contained ParameterFloat */
    void setValue(Double timeStamp, Double value);

signals:

public slots:

private:

    //Double getOffset_(Double time);

    ParameterFloat * valueParam_;

    Double timeStamp_, offset_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MODULATOROBJECTFLOAT_H
