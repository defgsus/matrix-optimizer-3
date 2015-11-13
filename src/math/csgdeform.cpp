#include <QObject> // for tr()

#include "csgdeform.h"
#include "types/properties.h"
#include "math/constants.h"

namespace MO {

// ################################# CsgRepeat ################################

MO_REGISTER_CSG(CsgRepeat)

CsgRepeat::CsgRepeat()
    : CsgDeformBase()
{
    props().set("repeat_x", QObject::tr("repeat X"), QObject::tr("repeat space on X axis"), true);
    props().set("repeat_y", QObject::tr("repeat Y"), QObject::tr("repeat space on Y axis"), false);
    props().set("repeat_z", QObject::tr("repeat Z"), QObject::tr("repeat space on Z axis"), false);
    props().set("min_x", QObject::tr("min X"), QObject::tr("Left side of repeat area"), -2., .1);
    props().set("max_x", QObject::tr("max X"), QObject::tr("Right side of repeat area"), 2., .1);
    props().set("min_y", QObject::tr("min Y"), QObject::tr("Bottom of repeat area"), -2., .1);
    props().set("max_y", QObject::tr("max Y"), QObject::tr("Top of repeat area"), 2., .1);
    props().set("min_z", QObject::tr("min Z"), QObject::tr("Back side of repeat area"), -2., .1);
    props().set("max_z", QObject::tr("max Z"), QObject::tr("Front side of repeat area"), 2., .1);
    props().setUpdateVisibilityCallback([](Properties& p)
    {
        bool v = p.get("repeat_x").toBool();
        p.setVisible("min_x", v);
        p.setVisible("max_x", v);
        v = p.get("repeat_y").toBool();
        p.setVisible("min_y", v);
        p.setVisible("max_y", v);
        v = p.get("repeat_z").toBool();
        p.setVisible("min_z", v);
        p.setVisible("max_z", v);
    });
}

QString CsgRepeat::getGlslFunctionBody() const
{
    if (numChildren() != 1)
        return QString();

    QString call = children().front()->glslCall();
    if (call.isEmpty())
        return call;

    Vec3 mi = Vec3(props().get("min_x").toFloat(),
                   props().get("min_y").toFloat(),
                   props().get("min_z").toFloat()),
         ma = Vec3(props().get("max_x").toFloat(),
                   props().get("max_y").toFloat(),
                   props().get("max_z").toFloat()),
         mo = ma - mi,
         hmo = mo / 2.f;

    QString swizz;
    if (props().get("repeat_x").toBool())
        swizz += "x";
    else
        mi.x = mo.x = hmo.x = 0.f;
    if (props().get("repeat_y").toBool())
        swizz += "y";
    else
        mi.y = mo.y = hmo.y = 0.f;
    if (props().get("repeat_z").toBool())
        swizz += "z";
    else
        mi.z = mo.z = hmo.z = 0.f;

    // no repeat selected
    // then getGlsl() will handle this
    if (swizz.isEmpty())
        return swizz;

    return QString("pos.%1 = mod(pos.%1 + %5.%1, %2.%1) - %3.%1;\n"
                   "return %4;")
                    .arg(swizz)
                    .arg(toGlsl(mo))
                    .arg(toGlsl(hmo))
                    .arg(call)
                    .arg(toGlsl(mi));
}

QString CsgRepeat::getGlsl() const
{
    if (numChildren() != 1)
        return QString();

    // pass children call through in case of no-repeat
    if (   !props().get("repeat_x").toBool()
        && !props().get("repeat_y").toBool()
        && !props().get("repeat_z").toBool())
        return children().front()->glslCall();

    return QString();
}





// ################################# CsgFan ################################

MO_REGISTER_CSG(CsgFan)

CsgFan::CsgFan()
    : CsgDeformBase()
{
    props().set("ang_start", QObject::tr("start angle"), QObject::tr("Starting angle in degree"), -15.);
    props().set("ang_end", QObject::tr("end angle"), QObject::tr("End angle in degree"), 15.);
}

QString CsgFan::getGlslFunctionBody() const
{
    if (numChildren() != 1)
        return QString();

    QString call = children().front()->glslCall();
    if (call.isEmpty())
        return call;

    float start = DEG_TO_TWO_PI * props().get("ang_start").toDouble(),
          end = DEG_TO_TWO_PI * props().get("ang_end").toDouble(),
          len = end - start;

    return QString(
        "float ang = atan(pos.x, pos.y), \n"
        "      len = length(pos.xy);\n"
        "ang = mod(ang - %1, %2) - %3;\n"
        "pos.xy = len * vec2(sin(ang), cos(ang));\n"
        "return %4;\n")
                    .arg(toGlsl(start))
                    .arg(toGlsl(len))
                    .arg(toGlsl(len/2.))
                    .arg(call);
}

QString CsgFan::getGlsl() const { return QString(); }


} // namespace MO
