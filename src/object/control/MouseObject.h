/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/26/2015</p>
*/

#ifndef MOSRC_OBJECT_CONTROL_MOUSEOBJECT_H
#define MOSRC_OBJECT_CONTROL_MOUSEOBJECT_H

#include "object/Object.h"
#include "object/interface/ValueFloatInterface.h"

namespace MO {

class MouseObject : public Object, public ValueFloatInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(MouseObject);

    Type type() const { return T_CONTROL; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual QString getOutputName(SignalType, uint channel) const Q_DECL_OVERRIDE;

    // ---- float interface ----

    Double valueFloat(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CONTROL_MOUSEOBJECT_H
