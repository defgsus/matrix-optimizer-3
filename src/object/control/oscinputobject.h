/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/13/2015</p>
*/

#ifndef MOSRC_OBJECT_CONTROL_OSCINPUTOBJECT_H
#define MOSRC_OBJECT_CONTROL_OSCINPUTOBJECT_H

#include "object/object.h"
#include "object/interface/valuefloatinterface.h"

namespace MO {

class OscInputObject : public Object, public ValueFloatInterface
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(OscInputObject);
    ~OscInputObject();

    Type type() const { return T_CONTROL; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual QString getOutputName(SignalType, uint channel) const Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    // ---- float interface ----

    Double valueFloat(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

signals:

private slots:

    void onValueChanged(const QString& id);

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CONTROL_OSCINPUTOBJECT_H
