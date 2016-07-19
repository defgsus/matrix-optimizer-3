/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/27/2016</p>
*/

// XXX Breaks history lookup
#if 0

#ifndef MOSRC_OBJECT_CONTROL_COUNTERCO_H
#define MOSRC_OBJECT_CONTROL_COUNTERCO_H

#include <QObject>

#include "object/object.h"
#include "object/interface/valuefloatinterface.h"

namespace MO {

class CounterCO
        : public Object
        , public ValueFloatInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(CounterCO);

    Type type() const { return T_CONTROL; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual QString getOutputName(SignalType, uint channel) const Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // ---- float interface ----

    Double valueFloat(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CONTROL_COUNTERCO_H

#endif
