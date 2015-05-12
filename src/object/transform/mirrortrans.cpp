/** @file mirrortrans.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#include "mirrortrans.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "math/vector.h"
#include "io/datastream.h"
//#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(MirrorTrans)

MirrorTrans::MirrorTrans(QObject *parent) :
    Transformation(parent)
{
    setName("Mirror");
}

void MirrorTrans::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("mirr", 1);
}

void MirrorTrans::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("mirr", 1);
}


void MirrorTrans::createParameters()
{
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        const QString axisTip = tr("Unit vector describing the normal of the mirror plane (internally normalized)");
        ax_ = params()->createFloatParameter("nx", "plane normal x", axisTip, 0.);
        ay_ = params()->createFloatParameter("ny", "plane normal y", axisTip, 1.);
        az_ = params()->createFloatParameter("nz", "plane normal z", axisTip, 0.);

        const QString transTip = tr("Origin of the mirror plane");
        x_ = params()->createFloatParameter("x", "center x", transTip, 0., 0.1);
        y_ = params()->createFloatParameter("y", "center y", transTip, 0., 0.1);
        z_ = params()->createFloatParameter("z", "center z", transTip, 0., 0.1);

    params()->endParameterGroup();
}


void MirrorTrans::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    // reflection vector
    Vec3 v = MATH::normalize_safe(Vec3(ax_->value(time, thread),
                                       ay_->value(time, thread),
                                       az_->value(time, thread)));
    // position
    Vec3 pos = Vec3(x_->value(time, thread),
                    y_->value(time, thread),
                    z_->value(time, thread));

    // orientation
    for (int i=0; i<3; ++i)
    {
        Vec3 p = glm::reflect( Vec3(matrix[i]), v );

        matrix[i][0] = p.x;
        matrix[i][1] = p.y;
        matrix[i][2] = p.z;
    }


    // position
    Vec3 p = glm::reflect( Vec3(matrix[3]) - pos, v ) + pos;

    matrix[3][0] = p.x;
    matrix[3][1] = p.y;
    matrix[3][2] = p.z;

    //MO_PRINT(matrix);
}


} // namespace MO

