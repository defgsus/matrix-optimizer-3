/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/15/2015</p>
*/

#ifndef MOSRC_MATH_CSGFRACTALS_H
#define MOSRC_MATH_CSGFRACTALS_H

#include "csgbase.h"

namespace MO {

class CsgApollonian : public CsgPositionSignedBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgApollonian, T_SOLID)
    QString getGlslFunctionBody() const override;
};

} // namespace MO

#endif // MOSRC_MATH_CSGFRACTALS_H
