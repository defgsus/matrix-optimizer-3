/** @file floatderivativeobject.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.04.2015</p>
*/

#ifndef MOSRC_OBJECT_CONTROL_DERIVATIVEOBJECTFLOAT_H
#define MOSRC_OBJECT_CONTROL_DERIVATIVEOBJECTFLOAT_H

#include "object/Object.h"
#include "object/interface/ValueFloatInterface.h"

namespace MO {

/** An object calculating the derivative of it's input */
class DerivativeObjectFloat
                : public Object
                , public ValueFloatInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(DerivativeObjectFloat);

    virtual Type type() const Q_DECL_OVERRIDE { return T_CONTROL; }

    virtual void createParameters() Q_DECL_OVERRIDE;

    Double valueFloat(uint channel, const RenderTime& time) const Q_DECL_OVERRIDE;

private:

    enum DifferentiationMethod
    {
        DM_APPROX = 1,
        /** https://en.wikipedia.org/wiki/Five-point_stencil */
        DM_FIVE
    };

    ParameterFloat
            * p_value_,
            * p_amp_,
            * p_range_,
            * p_offset_;
    ParameterSelect
            * p_method_;
    ParameterInt
            * p_order_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CONTROL_DERIVATIVEOBJECTFLOAT_H
