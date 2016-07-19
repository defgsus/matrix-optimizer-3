/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/24/2016</p>
*/

#include "ValueFloatInterface.h"
#include "math/random.h"
#include "io/log.h"

namespace MO {

void ValueFloatInterface::getValueFloatRange(
                    uint channel, const RenderTime &time, Double length,
                    Double *minimum, Double *maximum) const
{
    *minimum = *maximum = valueFloat(channel, time);

    const int num = std::min(5000, int(length * 10));
    if (length <= 0)
        return;

    for (int i=0; i<num; ++i)
    {
        const Double
                t = (i+MATH::rnd(0.,2.)) / num * length,
                val = valueFloat(channel, time + t);
        *minimum = std::min(*minimum, val);
        *maximum = std::max(*maximum, val);
    }

}

} // namespace MO
