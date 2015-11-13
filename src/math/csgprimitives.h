#ifndef MOSRC_MATH_CSGPRIMITIVES_H
#define MOSRC_MATH_CSGPRIMITIVES_H

#include "csgbase.h"

namespace MO {

class CsgPlane : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgPlane, T_SOLID)
};

class CsgSphere : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgSphere, T_SOLID)
};

class CsgCylinder : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgCylinder, T_SOLID)
};

class CsgBox : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgBox, T_SOLID)
    QString getGlslFunctionBody() const override;
};

class CsgTorus : public CsgPositionBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgTorus, T_SOLID)
};

} // namespace MO

#endif // MOSRC_MATH_CSGPRIMITIVES_H
