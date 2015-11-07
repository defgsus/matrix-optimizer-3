#ifndef MOSRC_MATH_CSGCOMBINE_H
#define MOSRC_MATH_CSGCOMBINE_H

#include "csgbase.h"

namespace MO {

class CsgCombineBase : public CsgBase
{
public:
    CsgCombineBase() : CsgBase() { }
    virtual bool canHaveChildren() const override { return true; }
};

class CsgUnion : public CsgCombineBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgUnion, T_COMBINER)
    virtual QString getGlslFunctionBody() const override;
};

} // namespace MO


#endif // MOSRC_MATH_CSGCOMBINE_H
