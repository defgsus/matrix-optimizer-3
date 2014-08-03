/** @file lookat.cpp

    @brief Look-at transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "lookat.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "io/datastream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(LookAt)

LookAt::LookAt(QObject *parent) :
    Transformation(parent)
{
    setName("LookAt");
}

void LookAt::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("lookat", 1);
}

void LookAt::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("lookat", 1);
}


void LookAt::createParameters()
{
    Transformation::createParameters();

    const QString lookTip = tr("Global position to look at"),
                  upTip = tr("unit vector describing the up-axis (internally normalized)");

    lookMode_ = createSelectParameter("lookmode", "look-at mode",
        tr("Selects if the look-at point is local or global"),
        { "local", "global" },
        { tr("local"), tr("global") },
        { tr("The look-at point is part of the transformation stack before the look-at transformation"),
          tr("The look-at point is global (world coordinates)") },
        { LM_LOCAL, LM_GLOBAL },
        LM_LOCAL,
        true, false
        );

    upMode_ = createSelectParameter("upmode", "up mode",
        tr("Selects if the up-axis is local or global"),
        { "local", "global" },
        { tr("local"), tr("global") },
        { tr("The up-axis point is part of the transformation stack before the look-at transformation"),
          tr("The up-axis point is global (world coordinates)") },
        { LM_LOCAL, LM_GLOBAL },
        LM_LOCAL,
        true, false
        );

    x_ = createFloatParameter("x", "look-at x", lookTip, 0);
    y_ = createFloatParameter("y", "look-at y", lookTip, 0);
    z_ = createFloatParameter("z", "look-at z", lookTip, 0);
    upX_ = createFloatParameter("upx", "up x", upTip, 0);
    upY_ = createFloatParameter("upy", "up y", upTip, 1);
    upZ_ = createFloatParameter("upz", "up z", upTip, 0);
}


void LookAt::applyTransformation(Mat4 &matrix, Double time, uint thread) const
{
    Vec3 lookAt = Vec3(x_->value(time, thread),
                       y_->value(time, thread),
                       z_->value(time, thread));
    Vec3 up = Vec3(upX_->value(time, thread),
                   upY_->value(time, thread),
                   upZ_->value(time, thread));

    bool glook = lookMode_->baseValue() == LM_GLOBAL,
         gup = upMode_->baseValue() == LM_GLOBAL;

    if (glook || gup)
    {
        Mat4 imatrix = glm::inverse(matrix);
        if (glook)
        {
            Vec4 v4 = imatrix * Vec4(lookAt[0], lookAt[1], lookAt[2], 1.0);
            lookAt[0] = v4[0];
            lookAt[1] = v4[1];
            lookAt[2] = v4[2];
        }
        if (gup)
        {
            Vec4 v4 = imatrix * Vec4(up[0], up[1], up[2], 0.0);
            up[0] = v4[0];
            up[1] = v4[1];
            up[2] = v4[2];
        }
    }

    // extract current position
    Vec3 pos = Vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

    // forward vector (look-at minus position)
    Vec3 f = glm::normalize(lookAt - pos);
    // up vector
    Vec3 u = glm::normalize(up);
    // right vector
    Vec3 s = glm::normalize(glm::cross(f, u));
    // rebuild up to avoid distortion
    u = glm::cross(s, f);

    Mat4 lookm(1);

    lookm[0][0] = s.x;
    lookm[0][1] = s.y;
    lookm[0][2] = s.z;
    lookm[1][0] = u.x;
    lookm[1][1] = u.y;
    lookm[1][2] = u.z;
    lookm[2][0] =-f.x;
    lookm[2][1] =-f.y;
    lookm[2][2] =-f.z;

    matrix *= lookm;
}


} // namespace MO

