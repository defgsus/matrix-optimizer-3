/** @file camerasettings.cpp

    @brief Settings for a camera for one projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/28/2014</p>
*/

#include "CameraSettings.h"
#include "io/XmlStream.h"
#include "math/vector.h"

namespace MO {

CameraSettings::CameraSettings()
    : width_    (1920),
      height_   (1080),
      fov_      (60),
      posX_     (0),
      posY_     (0),
      posZ_     (0),
      pitch_    (0),
      yaw_      (0),
      roll_     (0),
      zNear_    (0.01),
      zFar_     (10000.0)
{
}

void CameraSettings::serialize(IO::XmlStream & io) const
{
    io.createSection("camera");

        io.write("version", 1);
        io.write("width", width_);
        io.write("height", height_);
        io.write("fov", fov_);
        io.write("x", posX_);
        io.write("y", posY_);
        io.write("z", posZ_);
        io.write("pitch", pitch_);
        io.write("yaw", yaw_);
        io.write("roll", roll_);
        io.write("znear", zNear_);
        io.write("zfar", zFar_);

    io.endSection();
}

void CameraSettings::deserialize(IO::XmlStream & io)
{
    io.verifySection("camera");

        //int ver = io.expectInt("version");
        width_ = io.expectInt("width");
        height_ = io.expectInt("height");
        fov_ = io.expectFloat("fov");
        posX_ = io.expectFloat("x");
        posY_ = io.expectFloat("y");
        posZ_ = io.expectFloat("z");
        pitch_ = io.expectFloat("pitch");
        yaw_ = io.expectFloat("yaw");
        roll_ = io.expectFloat("roll");
        zNear_ = io.expectFloat("znear");
        zFar_ = io.expectFloat("zfar");
}

bool CameraSettings::operator == (const CameraSettings& o) const
{
    return width_ == o.width_
            && height_ == o.height_
            && fov_ == o.fov_
            && posX_ == o.posX_
            && posY_ == o.posY_
            && posZ_ == o.posZ_
            && pitch_ == o.pitch_
            && yaw_ == o.yaw_
            && roll_ == o.roll_
            && zNear_ == o.zNear_
            && zFar_ == o.zFar_;
}

Mat4 CameraSettings::getProjectionMatrix() const
{
    const Float aspect = Float(width_) / height_;
    return MATH::perspective(fov_, aspect, zNear_, zFar_);
}

Mat4 CameraSettings::getViewMatrix(Float pitch_offset) const
{
    Mat4 m(glm::translate(Mat4(1), pos()));

    // roll-pitch-yaw
    m = MATH::rotate(m, yaw_,   Vec3(0,1,0));
    m = MATH::rotate(m, pitch_ + pitch_offset, Vec3(1,0,0));
    m = MATH::rotate(m, roll_,  Vec3(0,0,1));

    return glm::inverse(m);
}

} // namespace MO
