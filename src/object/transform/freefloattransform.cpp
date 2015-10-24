/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/21/2015</p>
*/

#include "freefloattransform.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/parametercallback.h"
#include "object/param/parameterselect.h"
#include "math/vector.h"
#include "geom/freecamera.h"
#include "io/datastream.h"
//#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(FreeFloatTransform)

FreeFloatTransform::FreeFloatTransform(QObject *parent)
    : Transformation    (parent)
    , freefloat_        (new FreeFloatCamera)
    , doReset_          (true)
{
    setName("FreeFloat");

    freefloat_->camera().setInverse(true);
}

FreeFloatTransform::~FreeFloatTransform()
{
    delete freefloat_;
}

void FreeFloatTransform::serialize(IO::DataStream & io) const
{
    Transformation::serialize(io);
    io.writeHeader("freet", 1);
}

void FreeFloatTransform::deserialize(IO::DataStream & io)
{
    Transformation::deserialize(io);
    io.readHeader("freet", 1);
}


void FreeFloatTransform::createParameters()
{
    Transformation::createParameters();

    params()->beginParameterGroup("trans", tr("transformation"));
    initParameterGroupExpanded("trans");

        acc_x_ = params()->createFloatParameter(
                "acc_x", tr("acc. x right/left"),
                tr("X acceleration, + right, - left"),
                0.0, 0.01, true, true);

        acc_y_ = params()->createFloatParameter(
                "acc_y", tr("acc. y up/down"),
                tr("X acceleration, + up, - down"),
                0.0, 0.01, true, true);

        acc_z_ = params()->createFloatParameter(
                "acc_z", tr("acc. z for-/backward"),
                tr("Z acceleration, + forward, - backward"),
                0.0, 0.01, true, true);

        rx_ =   params()->createFloatParameter(
                "rot_x", tr("rotation x pitch"),
                tr("Rotation on X axis, + up, - down"),
                0.0, 0.01, true, true);

        ry_ =   params()->createFloatParameter(
                "rot_y", tr("rotation y yaw"),
                tr("Rotation on Y axis, + right, - left"),
                0.0, 0.01, true, true);

        rz_ =   params()->createFloatParameter(
                "rot_z", tr("rotation z roll"),
                tr("Rotation on Z axis, + right roll, - left roll"),
                0.0, 0.01, true, true);

        vel_ =  params()->createFloatParameter(
                "velocity", tr("velocity"),
                tr("Movement per time"),
                .5, 0.01, true, true);

        velr_ = params()->createFloatParameter(
                "rot_velocity", tr("velocity rotation"),
                tr("Rotation per time"),
                .2, 0.01, true, true);

        damp_ = params()->createFloatParameter(
                "friction", tr("friction"),
                tr("Damping factor"),
                10., 0.1, true, true);

        reset_ = params()->createCallbackParameter(
                    "reset", tr("reset"),
                    tr("Resets the matrix to zero"),
                    [this](){ doReset_ = true; }, true);

    params()->endParameterGroup();
}

void FreeFloatTransform::updateParameterVisibility()
{
    Transformation::updateParameterVisibility();

}

void FreeFloatTransform::applyTransformation(Mat4 &matrix, const RenderTime& time) const
{
    reset_->fireIfInput(time);

    if (doReset_)
    {
        freefloat_->setMatrix(Mat4(1.));
        doReset_ = false;
    }

    const Float
            acc_x = acc_x_->value(time),
            acc_y = acc_y_->value(time),
            acc_z = acc_z_->value(time),
            rot_x = rx_->value(time),
            rot_y = ry_->value(time),
            rot_z = rz_->value(time);

    freefloat_->moveX(acc_x);
    freefloat_->moveY(acc_y);
    freefloat_->moveZ(-acc_z);

    freefloat_->rotateX(rot_x);
    freefloat_->rotateY(-rot_y);
    freefloat_->rotateZ(rot_z);

    freefloat_->applyVelocity(time.delta() * vel_->value(time),
                              time.delta() * velr_->value(time));
    freefloat_->applyDamping(time.delta() * damp_->value(time));

    matrix *= freefloat_->getMatrix();
}


} // namespace MO
