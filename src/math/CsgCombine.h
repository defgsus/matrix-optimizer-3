#ifndef MOSRC_MATH_CSGCOMBINE_H
#define MOSRC_MATH_CSGCOMBINE_H

#include "CsgBase.h"

namespace MO {

class CsgCombineBase : public CsgBase
{
public:
    CsgCombineBase() : CsgBase() { }
    virtual int canHaveNumChildren() const override { return -1; }
};

class CsgUnion : public CsgCombineBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgUnion, T_COMBINE)
    virtual QString getGlslFunctionBody() const override;
};

class CsgIntersection : public CsgCombineBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgIntersection, T_COMBINE)
    virtual QString getGlslFunctionBody() const override;
};

class CsgDifference : public CsgCombineBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgDifference, T_COMBINE)
    virtual QString getGlslFunctionBody() const override;
};

class CsgBlob: public CsgCombineBase
{
public:
    MO_CSG_CONSTRUCTOR(CsgBlob, T_COMBINE)
    virtual QString getGlslFunctionBody() const override;
    virtual QString globalFunctions() const override;
private:
    QString sminGlsl_(const QString& arg1, const QString& arg2) const;
};

} // namespace MO


#endif // MOSRC_MATH_CSGCOMBINE_H
