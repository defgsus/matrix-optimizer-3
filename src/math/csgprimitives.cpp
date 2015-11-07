#include <QObject> // for tr()

#include "csgprimitives.h"
#include "types/properties.h"
#include "math/vector.h"

namespace MO {

/*
 * d = min(d, length(pos - vec3(1,2,3)) - 1);
 *
 *
 */


// ############################ CsgSphere ################################

MO_REGISTER_CSG(CsgSphere)

CsgSphere::CsgSphere()
    : CsgPositionBase()
{
    props().set("radius", QObject::tr("radius"), QObject::tr("The radius of the sphere"), 1.);
}

QString CsgSphere::getGlsl() const
{
    return QString("%1length(%2) - %3")
            .arg(glslComment())
            .arg(positionGlsl())
            .arg(toGlsl(props().get("radius").toDouble()))
            ;
}



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


} // namespace MO

