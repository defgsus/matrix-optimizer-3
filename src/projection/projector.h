/** @file

    @brief Projector/projection settings

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTOR_H
#define MOSRC_PROJECTION_PROJECTOR_H

#include "types/vector.h"

namespace MO {

/** Settings of one single projector */
class Projector
{
public:

    Projector();

    // ---------- getter ----------

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

    void width(int v) { width_ = v; recalc_(); }
    void height(int v) { height_ = v; recalc_(); }
    void fov(Float v) { fov_ = v; recalc_(); }
    void latitude(Float v) { latitude_ = v; recalc_(); }
    void longitude(Float v) { longitude_ = v; recalc_(); }
    void radius(Float v) { radius_ = v; recalc_(); }
    void pitch(Float v) { pitch_ = v; recalc_(); }
    void yaw(Float v) { yaw_ = v; recalc_(); }
    void roll(Float v) { roll_ = v; recalc_(); }

    // ----- getter for calculated values -----

    /** Are settings valid?
        If not, the calculated values may not make sense. */
    bool valid() const { return valid_; }

    /** Aspect ratio of projector */
    Float aspect() const { return aspect_; }

    /** Position of projector lens in 3d space. */
    const Vec3& pos() const { return pos_; }

    /** Returns the matrix of projection. */
    const Mat4& projectionMatrix() const { return rpy_; }

    /** Sphere coordinates for the given pixel */
    Vec2 mapToSphere(int pixel_x, int pixel_y) const;

    //______________ PRIVATE AREA _________________
private:

    void recalc_();

    // ---------- config ----------

    int width_, height_;
    Float
        fov_,
        latitude_,
        longitude_,
        radius_,
        pitch_,
        yaw_,
        roll_;

    // -------- calculated --------

    bool valid_;
    Float aspect_;
    Vec3 pos_;
    Mat4 rpy_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTOR_H
