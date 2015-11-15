#ifndef MOSRC_MATH_CSGDEFORM_H
#define MOSRC_MATH_CSGDEFORM_H

#include "csgbase.h"

namespace MO {

class CsgDeformBase : public CsgBase
{
public:
    CsgDeformBase() : CsgBase() { }
    virtual int canHaveNumChildren() const override { return 1; }
};


class CsgRepeat : public CsgDeformBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgRepeat, T_DEFORM)
    virtual QString getGlslFunctionBody() const override;
};

class CsgFan : public CsgDeformBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgFan, T_DEFORM)
    virtual QString getGlslFunctionBody() const override;
};

class CsgKaliFold : public CsgDeformBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgKaliFold, T_DEFORM)
    virtual QString getGlslFunctionBody() const override;
};


} // namespace MO

#endif // MOSRC_MATH_CSGDEFORM_H
