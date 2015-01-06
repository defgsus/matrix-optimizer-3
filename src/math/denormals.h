/** @file denormals.h

    @brief Denormals handling

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_MATH_DENORMALS_H
#define MOSRC_MATH_DENORMALS_H

namespace MO {
namespace MATH {

/** Globally enables or disables denormals (per thread).
    When disabled, denormals will be rounded to zero on usage. */
void setDenormals(bool enable);

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_DENORMALS_H
