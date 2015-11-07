#ifndef MOSRC_MATH_CSGPRIMITIVES_H
#define MOSRC_MATH_CSGPRIMITIVES_H

#include "csgbase.h"

namespace MO {

class CsgSphere : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgSphere, T_SOLID)
};

class CsgPlane : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgPlane, T_SOLID)
};

} // namespace MO

#endif // MOSRC_MATH_CSGPRIMITIVES_H
