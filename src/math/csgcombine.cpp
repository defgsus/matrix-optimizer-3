#include <QObject> // for tr()

#include "csgcombine.h"
#include "types/properties.h"

namespace MO {


// ################################# CsgUnion ################################

MO_REGISTER_CSG(CsgUnion)

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
        auto call = c->glslCall();
        if (!call.isEmpty())
            s += QString("d = min(d, %1);\n").arg(call);
    }

    return s;
}

QString CsgUnion::getGlsl() const
{
    if (numChildren() == 0)
        return QString();

    if (numChildren() == 1)
        return children().front()->glslCall();

    auto call1 = children()[0]->glslCall(),
         call2 = children()[1]->glslCall();
    if (call1.isEmpty())
    {
        if (call2.isEmpty())
            return QString();
        return call2;
    }
    else
    {
        if (call2.isEmpty())
            return call1;
        return QString("%1min(%2, %3)").arg(glslComment()).arg(call1).arg(call2);
    }
}



// ################################# CsgIntersection ################################

MO_REGISTER_CSG(CsgIntersection)

CsgIntersection::CsgIntersection()
    : CsgCombineBase()
{

}

QString CsgIntersection::getGlslFunctionBody() const
{
    if (numChildren() <= 2)
        return QString();

    QString s;
    for (auto c : children())
    {
        auto call = c->glslCall();
        if (!call.isEmpty())
            s += QString("d = max(d, %1);\n").arg(call);
    }

    return s;
}

QString CsgIntersection::getGlsl() const
{
    if (numChildren() == 0)
        return QString();

    if (numChildren() == 1)
        return children().front()->glslCall();


    auto call1 = children()[0]->glslCall(),
         call2 = children()[1]->glslCall();
    if (call1.isEmpty())
    {
        if (call2.isEmpty())
            return QString();
        return call2;
    }
    else
    {
        if (call2.isEmpty())
            return call1;
        return QString("%1max(%2, %3)").arg(glslComment()).arg(call1).arg(call2);
    }
}





// ################################# CsgDifference ################################

MO_REGISTER_CSG(CsgDifference)

CsgDifference::CsgDifference()
    : CsgCombineBase()
{

}

QString CsgDifference::getGlslFunctionBody() const
{
    if (numChildren() <= 2)
        return QString();

    bool isFirst = true;
    QString s;
    for (auto c : children())
    {
        auto call = c->glslCall();
        if (call.isEmpty())
            continue;

        if (isFirst)
            s += QString("float d = %1;\n").arg(call);
        else
            s += QString("d = max(d, -(%1));\n").arg(call);

        isFirst = false;
    }

    return s;
}

QString CsgDifference::getGlsl() const
{
    if (numChildren() == 0)
        return QString();

    if (numChildren() == 1)
        return children().front()->glslCall();


    auto call1 = children()[0]->glslCall(),
         call2 = children()[1]->glslCall();
    if (call1.isEmpty())
    {
        if (call2.isEmpty())
            return QString();
        return call2;
    }
    else
    {
        if (call2.isEmpty())
            return call1;
        return QString("%1max(%2, -(%3))").arg(glslComment()).arg(call1).arg(call2);
    }
}










// ################################# CsgBlob ################################

MO_REGISTER_CSG(CsgBlob)

CsgBlob::CsgBlob()
    : CsgCombineBase()
{
    Properties::NamedValues modes;
    modes.set("exp", QObject::tr("exponential"), QObject::tr("exponential smoothing"), 0);
    modes.set("poly", QObject::tr("polynomial"), QObject::tr("polynomial smoothing"), 1);
    modes.set("pow", QObject::tr("power"), QObject::tr("power smoothing"), 2);
    props().set("mode", QObject::tr("mode"), QObject::tr("The kind of smoothing function"), modes, 0);

    props().set("exp_k", QObject::tr("smoothness"), QObject::tr("Smoothing factor"), 32., .1);
    props().set("poly_k", QObject::tr("smoothness"), QObject::tr("Smoothing factor"), .1, .01);
    props().set("pow_k", QObject::tr("smoothness"), QObject::tr("Smoothing factor"), 8., .1);

    props().setUpdateVisibilityCallback([](Properties& p)
    {
        int mode = p.get("mode").toInt();
        p.setVisible("exp_k", mode == 0);
        p.setVisible("poly_k", mode == 1);
        p.setVisible("pow_k", mode == 2);
    });
}

QString CsgBlob::globalFunctions() const
{
    return
        "float smooth_min_exp(float a, float b, float k)\n"
        "{\n"
        "\tfloat res = exp(-k * a) + exp(-k * b);\n"
        "\treturn -log(res) / k;\n"
        "}\n"
        "\n"
        "float smooth_min_poly(float a, float b, float k)\n"
        "{\n"
        "\tfloat h = clamp(.5 + .5*(b - a) / k, 0., 1.);\n"
        "\treturn mix(b, a, h) - k * h * (1. - h);\n"
        "}\n"
        "\n"
        "float smooth_min_pow(float a, float b, float k)\n"
        "{\n"
        "\ta = pow(a, k); b = pow(b, k);\n"
        "\treturn pow((a * b) / (a + b), 1. / k);\n"
        "}\n";
}

QString CsgBlob::sminGlsl_(const QString& arg1, const QString& arg2) const
{
    switch (props().get("mode").toInt())
    {
        default:
        case 0: return QString("smooth_min_exp(%1, %2, %3)")
                        .arg(arg1).arg(arg2).arg(toGlsl(props().get("exp_k").toDouble()));
        case 1: return QString("smooth_min_poly(%1, %2, %3)")
                        .arg(arg1).arg(arg2).arg(toGlsl(props().get("poly_k").toDouble()));
        case 2: return QString("smooth_min_pow(%1, %2, %3)")
                        .arg(arg1).arg(arg2).arg(toGlsl(props().get("pow_k").toDouble()));
    }
}

QString CsgBlob::getGlslFunctionBody() const
{
    if (numChildren() <= 2)
        return QString();

    QString s;
    for (auto c : children())
    {
        auto call = c->glslCall();
        if (!call.isEmpty())
            s += QString("d = %1;\n").arg(sminGlsl_("d", call));
    }

    return s;
}

QString CsgBlob::getGlsl() const
{
    if (numChildren() == 0)
        return QString();

    if (numChildren() == 1)
        return children().front()->glslCall();

    auto call1 = children()[0]->glslCall(),
         call2 = children()[1]->glslCall();
    if (call1.isEmpty())
    {
        if (call2.isEmpty())
            return QString();
        return call2;
    }
    else
    {
        if (call2.isEmpty())
            return call1;
        return QString("%1%2").arg(glslComment()).arg(sminGlsl_(call1, call2));
    }
}


} // namespace MO
