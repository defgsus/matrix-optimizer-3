/** @file projectorsettings.h

    @brief Settings of a single projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORSETTINGS_H
#define MOSRC_PROJECTION_PROJECTORSETTINGS_H

#include <QString>

#include "types/float.h"

namespace MO {

/** Settings of one single projector */
class ProjectorSettings
{
public:

    ProjectorSettings();

    // ---------- getter ----------

    const QString& name() const { return name_; }
    int width() const { return width_; }
    int height() const { return height_; }
    Float fov() const { return fov_; }
    Float latitude() const { return latitude_; }
    Float longitude() const { return longitude_; }
    Float radius() const { return radius_; }
    Float pitch() const { return pitch_; }
    Float yaw() const { return yaw_; }
    Float roll() const { return roll_; }

    // -------- setter ------------

    void setName(const QString& name) { name_ = name; }
    void setWidth(int v) { width_ = v; }
    void setHeight(int v) { height_ = v; }
    void setFov(Float v) { fov_ = v; }
    void setLatitude(Float v) { latitude_ = v; }
    void setLongitude(Float v) { longitude_ = v; }
    void setRadius(Float v) { radius_ = v; }
    void setPitch(Float v) { pitch_ = v; }
    void setYaw(Float v) { yaw_ = v; }
    void setRoll(Float v) { roll_ = v; }

    //______________ PRIVATE AREA _________________
private:

    // ---------- config ----------

    QString name_;
    int width_, height_;
    Float
        fov_,
        latitude_,
        longitude_,
        radius_,
        pitch_,
        yaw_,
        roll_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTORSETTINGS_H
