#include <QObject> // for tr()

#include "csgcombine.h"
#include "types/properties.h"

namespace MO {

// ################################# CsgUnion ################################

CsgUnion::CsgUnion()
    : CsgCombineBase()
{

}

QString CsgUnion::getGlslFunctionBody() const
{
    if (numChildren() <= 2)
        return QString();

    QString s;
    for (auto c : children())
    {
        s += QString("d = min(d, %2);\n")
                .arg(c->glslCall());
    }

    return s;
}

QString CsgUnion::getGlsl() const
{
    if (numChildren() == 0)
        return QString();

    if (numChildren() == 1)
        return children().front()->glslCall();

    return QString("%1min(%2, %3)")
        .arg(glslComment())
        .arg(children()[0]->glslCall())
        .arg(children()[1]->glslCall())
        ;
}


} // namespace MO
