/** @file cleartrans.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.05.2015</p>
*/

#include "cleartrans.h"
#include "object/param/parameters.h"
#include "object/param/parameterselect.h"
#include "io/datastream.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(ClearTrans)

ClearTrans::ClearTrans()
    : Transformation    ()
{
    setName("Clear");
}

void ClearTrans::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("clr", 1);
}

void ClearTrans::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("clr", 1);
}


void ClearTrans::createParameters()
{
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        cPos_ = params()->createBooleanParameter("clear_pos", tr("clear position"),
                                                 tr("Removes the position from the transformation"),
                                                 tr("The position is left alone"),
                                                 tr("The position is cleared"),
                                                 false, true, true);

        cScale_ = params()->createBooleanParameter("clear_scale", tr("clear scale"),
                                                 tr("Removes the scaling from the transformation"),
                                                 tr("The scale is left alone"),
                                                 tr("The scale is reset to 1"),
                                                 false, true, true);

        cRot_ = params()->createBooleanParameter("clear_rot", tr("clear rotation"),
                                                 tr("Removes the rotation from the transformation"),
                                                 tr("The rotation is left alone"),
                                                 tr("The rotation is cleared"),
                                                 false, true, true);

    params()->endParameterGroup();
}


void ClearTrans::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    const bool
            cPos = cPos_->value(time),
            cRot = cRot_->value(time),
            cScale = cScale_->value(time);

    if (cPos)
    {
        matrix[3][0] = matrix[3][1] = matrix[3][2] = 0.f;
    }

    if (cRot)
    {
        // reset orientation completely
        if (cScale)
        {
            matrix[0][1] = matrix[0][2] =
            matrix[1][0] = matrix[1][2] =
            matrix[2][0] = matrix[2][1] = 0.f;
            matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.;
        }
        // keep scale, remove rotation
        else
        {
            const Float
                    l1 = glm::length(Vec3(matrix[0][0], matrix[0][1], matrix[0][2])),
                    l2 = glm::length(Vec3(matrix[1][0], matrix[1][1], matrix[1][2])),
                    l3 = glm::length(Vec3(matrix[2][0], matrix[2][1], matrix[2][2]));
            matrix[0][1] = matrix[0][2] =
            matrix[1][0] = matrix[1][2] =
            matrix[2][0] = matrix[2][1] = 0.f;
            matrix[0][0] = l1;
            matrix[1][1] = l2;
            matrix[2][2] = l3;
        }
    }
    else
    {
        // remove scale only
        if (cScale)
        {
            for (int i=0; i<3; ++i)
            {
                Vec3 v = glm::normalize(Vec3(matrix[i][0], matrix[i][1], matrix[i][2]));
                matrix[i][0] = v.x; matrix[i][1] = v.y; matrix[i][2] = v.z;
            }
        }
    }
}


} // namespace MO

