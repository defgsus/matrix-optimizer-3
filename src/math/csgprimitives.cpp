#include <QObject> // for tr()

#include "csgprimitives.h"
#include "types/properties.h"
#include "math/vector.h"

namespace MO {




// ############################ CsgPlane ################################

MO_REGISTER_CSG(CsgPlane)

CsgPlane::CsgPlane()
    : CsgPositionBase()
{
    props().set("nx", QObject::tr("normal X"), QObject::tr("X vector of the plane normal"), 0.);
    props().set("ny", QObject::tr("normal Y"), QObject::tr("Y vector of the plane normal"), 1.);
    props().set("nz", QObject::tr("normal Z"), QObject::tr("Z vector of the plane normal"), 0.);
}

QString CsgPlane::getGlsl() const
{
    Vec3 n = MATH::normalize_safe(Vec3(
                props().get("nx").toFloat(),
                props().get("ny").toFloat(),
                props().get("nz").toFloat()));

    return QString("%1dot(%2, %3)")
            .arg(glslComment())
            .arg(positionGlsl())
            .arg(toGlsl(n))
            ;
}





// ############################ CsgSphere ################################

MO_REGISTER_CSG(CsgSphere)

CsgSphere::CsgSphere()
    : CsgPositionBase()
{
    props().set("radius", QObject::tr("radius"), QObject::tr("The radius of the sphere"), 1., 0.1);
}

QString CsgSphere::getGlsl() const
{
    return QString("%1length(%2) - %3")
            .arg(glslComment())
            .arg(positionGlsl())
            .arg(toGlsl(props().get("radius").toDouble()))
            ;
}



// ############################ CsgCylinder ################################

MO_REGISTER_CSG(CsgCylinder)

CsgCylinder::CsgCylinder()
    : CsgPositionBase()
{
    Properties::NamedValues axis;
    axis.set("x", QObject::tr("X axis"), QObject::tr("X axis"), 0);
    axis.set("y", QObject::tr("Y axis"), QObject::tr("Y axis"), 1);
    axis.set("z", QObject::tr("Z axis"), QObject::tr("Z axis"), 2);
    props().set("axis", QObject::tr("axis"), QObject::tr("The axis along the cylinder is lying"), axis, 1);

    props().set("radius", QObject::tr("radius"), QObject::tr("The radius of the cylinder"), 1., 0.1);
}

QString CsgCylinder::getGlsl() const
{
    int axis = props().get("axis").toInt();
    QString swizz = "yz";
    if (axis == 1)
        swizz = "xz";
    else if (axis == 2)
        swizz = "xy";

    return QString("%1length((%2).%4) - %3")
            .arg(glslComment())
            .arg(positionGlsl())
            .arg(toGlsl(props().get("radius").toDouble()))
            .arg(swizz)
            ;
}



// ############################ CsgBox ################################

MO_REGISTER_CSG(CsgBox)

CsgBox::CsgBox()
    : CsgPositionBase()
{
    props().set("sizex", QObject::tr("size X"), QObject::tr("extend on X axis"), 1., 0.1);
    props().set("sizey", QObject::tr("size Y"), QObject::tr("extend on Y axis"), 1., 0.1);
    props().set("sizez", QObject::tr("size Z"), QObject::tr("extend on Z axis"), 1., 0.1);
}


QString CsgBox::getGlsl() const
{
    return QString();
}

QString CsgBox::getGlslFunctionBody() const
{
    Vec3 ext(props().get("sizex").toFloat(),
             props().get("sizey").toFloat(),
             props().get("sizez").toFloat());

    return
    QString("vec3 b = abs(%1) - %2;\n"
            "return min(max(b.x, max(b.y, b.z)), 0.0) + length(max(b, 0.0));")
            .arg(positionGlsl())
            .arg(toGlsl(ext))
            ;
}






// ############################ CsgTorus ################################

MO_REGISTER_CSG(CsgTorus)

CsgTorus::CsgTorus()
    : CsgPositionBase()
{
    Properties::NamedValues planes;
    planes.set("xy", QObject::tr("X-Y plane"), QObject::tr("X-Y plane"), 0);
    planes.set("xz", QObject::tr("X-Z plane"), QObject::tr("X-Z plane"), 1);
    planes.set("yz", QObject::tr("Y-Z plane"), QObject::tr("Y-Z plane"), 2);
    props().set("plane", QObject::tr("plane"), QObject::tr("The plane on which the torus lies"), planes, 0);

    props().set("radius", QObject::tr("radius"), QObject::tr("Radius of the torus"), 1., 0.1);
    props().set("radius2", QObject::tr("radius 2"), QObject::tr("Small radius of the torus"), .1, 0.1);

}


QString CsgTorus::getGlsl() const
{
    QString plane = "xy",
            ortho = "z";
    int pmode = props().get("plane").toInt();
    if (pmode == 1)
    {
        plane = "xz";
        ortho = "y";
    }
    else if (pmode == 2)
    {
        plane = "yz";
        ortho = "x";
    }

    return
    QString("%1length(vec2(length((%2).%3) - %5, (%2).%4)) - %6")
            .arg(glslComment())
            .arg(positionGlsl())
            .arg(plane)
            .arg(ortho)
            .arg(props().get("radius").toDouble())
            .arg(props().get("radius2").toDouble());
}





} // namespace MO

