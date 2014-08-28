/** @file camerasettings.h

    @brief Settings for a camera for one projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/28/2014</p>
*/

#ifndef MOSRC_PROJECTION_CAMERASETTINGS_H
#define MOSRC_PROJECTION_CAMERASETTINGS_H

#include "types/vector.h"

namespace MO {
namespace IO { class XmlStream; }

class CameraSettings
{
public:
    CameraSettings();

    // ------------ io ------------

    void serialize(IO::XmlStream&) const;
    void deserialize(IO::XmlStream&);

    // ---------------- getter -------------------

    bool operator == (const CameraSettings&) const;

    int   width() const { return width_; }
    int   height() const { return height_; }
    Float fov() const { return fov_; }
    Float posX() const { return posX_; }
    Float posY() const { return posY_; }
    Float posZ() const { return posZ_; }
    Vec3  pos() const { return Vec3(posX_, posY_, posZ_); }
    Float pitch() const { return pitch_; }
    Float yaw() const { return yaw_; }
    Float roll() const { return roll_; }
    Float zNear() const { return zNear_; }
    Float zFar() const { return zFar_; }

    Mat4 getPerspectiveMatrix() const;
    Mat4 getViewMatrix() const;

    // --------------- setter --------------------

    void setWidth(int v) { width_ = v; }
    void setHeight(int v) { height_ = v; }
    void setFov(Float v) { fov_ = v; }
    void setPosX(Float v) { posX_ = v; }
    void setPosY(Float v) { posY_ = v; }
    void setPosZ(Float v) { posZ_ = v; }
    void setPos(const Vec3& p) { posX_ = p[0]; posY_ = p[1]; posZ_ = p[2]; }
    void setPitch(Float v) { pitch_ = v; }
    void setYaw(Float v) { yaw_ = v; }
    void setRoll(Float v) { roll_ = v; }
    void setZNear(Float v) { zNear_ = v; }
    void setZFar(Float v) { zFar_ = v; }

private:

    int width_, height_;

    Float
        fov_,
        posX_,
        posY_,
        posZ_,
        pitch_,
        yaw_,
        roll_,
        zNear_,
        zFar_;
};


} // namespace MO


#endif // MOSRC_PROJECTION_CAMERASETTINGS_H
