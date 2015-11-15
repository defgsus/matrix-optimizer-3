/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/15/2015</p>
*/

#include <QObject> // for tr()

#include "csgfractals.h"
#include "types/properties.h"
#include "math/vector.h"

namespace MO {


// ############################ CsgApollonian ################################

MO_REGISTER_CSG(CsgApollonian)

CsgApollonian::CsgApollonian()
    : CsgPositionSignedBase()
{
    props().set("iterations", QObject::tr("iterations"), QObject::tr("Number of iterations"), 8u);
    props().set("factor", QObject::tr("factor"), QObject::tr("Radius/scale factor"), 1., 0.1);
}

QString CsgApollonian::getGlsl() const { return QString(); }

QString CsgApollonian::getGlslFunctionBody() const
{
    QString s = QString(
        "// Created by inigo quilez - iq/2013\n"
        "// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.\n"
        "// https://www.shadertoy.com/view/4ds3zn\n"
        "float scale = 1.;\n"
        "for (int i = 0; i < %1; ++i)\n"
        "{\n"
            "\tpos = -1. + 2. * fract(.5 * pos + .5);\n"
            "\tfloat r2 = dot(pos, pos);\n"
            "\tfloat k = max(%2 / r2, .1);\n"
            "\tpos *= k;\n"
            "\tscale *= k;\n"
        "}\n"
        "return .25 * abs(pos.y) / scale;\n"
        )
            .arg(props().get("iterations").toUInt())
            .arg(toGlsl(props().get("factor").toDouble()))
            ;

    return s;
}


} // namespace MO


