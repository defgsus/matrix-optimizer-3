/** @file floatderivativeobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.04.2015</p>
*/

#include "derivativeobjectfloat.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterint.h"
#include "object/param/parameterselect.h"
#include "math/interpol.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(DerivativeObjectFloat)

DerivativeObjectFloat::DerivativeObjectFloat()
    : Object        ()
{
    setName("DerivativeFloat");
    setNumberOutputs(ST_FLOAT, 1);
}

void DerivativeObjectFloat::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("derf", 1);
}

void DerivativeObjectFloat::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("derf", 1);
}

void DerivativeObjectFloat::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("modval", tr("value"));
    initParameterGroupExpanded("modval");

        p_value_ = params()->createFloatParameter("val", tr("value"),
                                       tr("Input float value from which the derivative is calculated"),
                                       0.0, 1.0);
        p_value_->setVisibleGraph(true);
        p_value_->setDefaultEvolvable(false);

        p_order_ = params()->createIntParameter("order", tr("order"),
                                        tr("Selects the order of differentiation"),
                                        1, 1, 4, 1,
                                        true, true);

        p_method_ = params()->createSelectParameter("method", tr("method"),
                                        tr("Selects the method of differentiation"),
                                        { "fda", "5p" },
                                        { tr("finite difference approximation"),
                                          tr("five-point stencil") },
                                        { tr("Simplest form of finite differentiation"),
                                          tr("More exact differentiation method using 5 taps"),
                                        },
                                        { DM_APPROX, DM_FIVE },
                                        DM_APPROX,
                                        true, false);

        p_amp_ = params()->createFloatParameter("amp", tr("amplitude"),
                                       tr("Output multiplier"),
                                       1.0, 0.1);

        p_range_ = params()->createFloatParameter("delta", tr("time delta"),
                                       tr("Time range in seconds over which the derivative is estimated"),
                                       .1, 0.1);
        p_range_->setMinValue(0.0000001);

        p_offset_ = params()->createFloatParameter("offset", tr("time offset"),
                                       tr("Time offset in seconds that is added to the lookup time"),
                                       0.0, 0.1);



    params()->endParameterGroup();
}

Double DerivativeObjectFloat::valueFloat(uint, const RenderTime& time) const
{
    DifferentiationMethod method = (DifferentiationMethod)p_method_->baseValue();
    int method_order = method * 10 + p_order_->value(time);

    const Double
            amp = p_amp_->value(time);
    const RenderTime ti = time + p_offset_->value(time);


    switch (method_order)
    {
        default:
        case 11:
        {
            const Double
                    h = p_range_->value(time),
                    h2 = .5 * h,
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti + h2);

            return amp * (d2 - d1) / h;
        }

        case 12:
        {
            const Double
                    h = p_range_->value(time),
                    h2 = .5 * h,
                    d0 = p_value_->value(ti     ),
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti + h2);

            return amp * (d2 - 2. * d0 + d1) / (h * h);
        }

        /** @todo add 3 and 4 order simple finite difference methods */

        // --------- 5-point-stencil -----------

        case 21:
        {
            const Double
                    r = p_range_->value(time),
                    h = .5 * r,
                    h2 = 2. * h,
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti - h ),
                    d3 = p_value_->value(ti + h ),
                    d4 = p_value_->value(ti + h2);

            return amp * ((-d4 + 8.*d3 - 8.*d2 + d1) / (12.*h)
                          // + r1*r1*r1*r1 / 30. * der5
                          );
        }

        case 22:
        {
            const Double
                    r = p_range_->value(time),
                    h = .5 * r,
                    h2 = 2. * h,
                    d0 = p_value_->value(ti     ),
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti - h ),
                    d3 = p_value_->value(ti + h ),
                    d4 = p_value_->value(ti + h2);

            return amp * (-d4 + 16.*d3 - 30.*d0 + 16.*d2 - d1) / (12.*h*h);
        }

        case 23:
        {
            const Double
                    r = p_range_->value(time),
                    h = .5 * r,
                    h2 = 2. * h,
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti - h ),
                    d3 = p_value_->value(ti + h ),
                    d4 = p_value_->value(ti + h2);

            return amp * (d4 - 2.*d3 + 2.*d2 - d1) / (2.*h*h*h);
        }

        case 24:
        {
            const Double
                    r = p_range_->value(time),
                    h = .5 * r,
                    h2 = 2. * h,
                    d0 = p_value_->value(ti     ),
                    d1 = p_value_->value(ti - h2),
                    d2 = p_value_->value(ti - h ),
                    d3 = p_value_->value(ti + h ),
                    d4 = p_value_->value(ti + h2);

            return amp * (d4 - 4.*d3 + 6.*d0 - 4.*d2 + d1) / (h*h*h*h);
        }
    }

}


} // namespace MO
