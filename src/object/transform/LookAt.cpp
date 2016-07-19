/** @file lookat.cpp

    @brief Look-at transformation object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/10/2014</p>
*/

#include "LookAt.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFloat.h"
#include "object/param/ParameterSelect.h"
#include "io/DataStream.h"
#include "math/vector.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(LookAt)

LookAt::LookAt()
    : Transformation()
{
    setName("LookAt");
}

LookAt::~LookAt() { }

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

    params()->beginParameterGroup("transmode", tr("transformation mode"));

        const QString lookTip = tr("Global position to look at"),
                      upTip = tr("unit vector describing the up-axis (internally normalized)");

        lookMode_ = params()->createSelectParameter("lookmode", "look-at mode",
            tr("Selects if the look-at point is local or global"),
            { "local", "global" },
            { tr("local"), tr("global") },
            { tr("The look-at point is part of the transformation stack before the look-at transformation"),
              tr("The look-at point is global (world coordinates)") },
            { LM_LOCAL, LM_GLOBAL },
            LM_LOCAL,
            true, false
            );

        upMode_ = params()->createSelectParameter("upmode", "up mode",
            tr("Selects if the up-axis is local or global"),
            { "local", "global" },
            { tr("local"), tr("global") },
            { tr("The up-axis point is part of the transformation stack before the look-at transformation"),
              tr("The up-axis point is global (world coordinates)") },
            { LM_LOCAL, LM_GLOBAL },
            LM_LOCAL,
            true, false
            );

    params()->endParameterGroup();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        x_ = params()->createFloatParameter("x", "look-at x", lookTip, 0);
        y_ = params()->createFloatParameter("y", "look-at y", lookTip, 0);
        z_ = params()->createFloatParameter("z", "look-at z", lookTip, 0);
        upX_ = params()->createFloatParameter("upx", "up x", upTip, 0);
        upY_ = params()->createFloatParameter("upy", "up y", upTip, 1);
        upZ_ = params()->createFloatParameter("upz", "up z", upTip, 0);

    params()->endParameterGroup();
}


void LookAt::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    Vec3 lookAt = Vec3(x_->value(time),
                       y_->value(time),
                       z_->value(time));
    Vec3 up = Vec3(upX_->value(time),
                   upY_->value(time),
                   upZ_->value(time));

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
    Vec3 f = lookAt - pos;
    // failsafe
    if (std::abs(f.x) < 0.00001 && std::abs(f.y) < 0.00001
            && std::abs(f.z) < 0.00001)
        f.z = -1;
    f = MATH::normalize_safe(f);
    // up vector
    Vec3 u = MATH::normalize_safe(up);
    // right vector
    Vec3 s = MATH::normalize_safe(glm::cross(f, u));
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

