/** @file mirrortrans.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#include "mirrortrans.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "math/vector.h"
#include "io/datastream.h"
//#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(MirrorTrans)

MirrorTrans::MirrorTrans()
    : Transformation()
{
    setName("Mirror");
}

MirrorTrans::~MirrorTrans() { }

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

        mPos_ = params()->createBooleanParameter("mirror_pos", tr("mirror position"),
                                                 tr("Enables mirroring of the position"),
                                                 tr("The position is left alone"),
                                                 tr("The position is mirrored as well"),
                                                 true, true, true);

        mOri_ = params()->createBooleanParameter("mirror_orientation", tr("mirror orientation"),
                                                 tr("Enables mirroring of the orientation"),
                                                 tr("The orientation is left alone"),
                                                 tr("The orientation is mirrored as well"),
                                                 true, true, true);

        const QString axisTip = tr("Unit vector describing the normal of the mirror plane (internally normalized)");
        ax_ = params()->createFloatParameter("nx", tr("plane normal x"), axisTip, 0.);
        ay_ = params()->createFloatParameter("ny", tr("plane normal y"), axisTip, 1.);
        az_ = params()->createFloatParameter("nz", tr("plane normal z"), axisTip, 0.);

        const QString transTip = tr("Origin of the mirror plane");
        x_ = params()->createFloatParameter("x", tr("center x"), transTip, 0., 0.1);
        y_ = params()->createFloatParameter("y", tr("center y"), transTip, 0., 0.1);
        z_ = params()->createFloatParameter("z", tr("center z"), transTip, 0., 0.1);

    params()->endParameterGroup();
}

void MirrorTrans::updateParameterVisibility()
{
    Transformation::updateParameterVisibility();

    const bool mpos = mPos_->baseValue();
    x_->setVisible(mpos);
    y_->setVisible(mpos);
    z_->setVisible(mpos);
}

void MirrorTrans::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    const bool
            mPos = mPos_->value(time),
            mOri = mOri_->value(time);

    if (!(mPos || mOri))
        return;

    // reflection vector
    Vec3 v = MATH::normalize_safe(Vec3(ax_->value(time),
                                       ay_->value(time),
                                       az_->value(time)));
    // orientation
    if (mOri)
    for (int i=0; i<3; ++i)
    {
        Vec3 p = glm::reflect( Vec3(matrix[i]), v );

        matrix[i][0] = p.x;
        matrix[i][1] = p.y;
        matrix[i][2] = p.z;
    }

    // position
    if (mPos)
    {
        // position
        Vec3 pos = Vec3(x_->value(time),
                        y_->value(time),
                        z_->value(time));

        Vec3 p = glm::reflect( Vec3(matrix[3]) - pos, v ) + pos;

        matrix[3][0] = p.x;
        matrix[3][1] = p.y;
        matrix[3][2] = p.z;
    }

    //MO_PRINT(matrix);
}


} // namespace MO

