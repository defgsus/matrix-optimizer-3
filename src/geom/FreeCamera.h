/** @file freecamera.h

    @brief Free floating camera transform

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/3/2014</p>
*/

#ifndef MOSRC_GEOM_FREECAMERA_H
#define MOSRC_GEOM_FREECAMERA_H

#include "types/vector.h"

namespace MO {


class FreeCamera
{
public:
    FreeCamera();

    // -------------- getter -----------------

    bool isInverse() const { return inverse_; }

    const Mat4& getMatrix() const { return matrix_; }

    /** Returns forward vector */
    Vec3 forward() const;

    // -------------- setter -----------------

    void setMatrix(const Mat4& );

    void setInverse(bool enable) { inverse_ = enable; }

    // ------------- navigation --------------

    /** Set absolute position */
    void setPosition(const Vec3&);

    /** Move forward/backward */
    void moveZ(Float steps);
    /** Move right/left */
    void moveX(Float steps);
    /** Move up/down */
    void moveY(Float steps);

    void move(const Vec3& steps);

    void rotateX(Float degree);
    void rotateY(Float degree);
    void rotateZ(Float degree);
private:

    Mat4 matrix_;
    bool inverse_;
};

/** A velocity based camera controller */
class FreeFloatCamera
{
public:
    FreeFloatCamera() { }

    // -------------- getter -----------------

    /** Access to internal camera */
    const FreeCamera& camera() const { return cam_; }
    FreeCamera& camera() { return cam_; }

    const Mat4& getMatrix() const { return cam_.getMatrix(); }

    /** Returns forward vector */
    Vec3 forward() const { return cam_.forward(); }

    // -------------- setter -----------------

    void setMatrix(const Mat4& m) { cam_.setMatrix(m); }

    // ------------- navigation --------------

    /** Set absolute position */
    void setPosition(const Vec3& p) { cam_.setPosition(p); }

    /** Move forward/backward */
    void moveZ(Float velo) { velo_.z += velo; }
    /** Move right/left */
    void moveX(Float velo) { velo_.x += velo; }
    /** Move up/down */
    void moveY(Float velo) { velo_.y += velo; }

    void rotateX(Float velo_degree) { veloRot_.x += velo_degree; }
    void rotateY(Float velo_degree) { veloRot_.y += velo_degree; }
    void rotateZ(Float velo_degree) { veloRot_.z += velo_degree; }

    // ----------- realtime ------------------

    void step(Float delta, Float force)
    {
        applyVelocity(delta * force);
        applyDamping(delta);
    }

    void applyVelocity(Float delta) { applyVelocity(delta, delta); }
    void applyVelocity(Float deltaVel, Float deltaRot);
    void applyDamping(Float delta);

private:

    FreeCamera cam_;

    Vec3 velo_, veloRot_;
};

} // namespace MO

#endif // MOSRC_GEOM_FREECAMERA_H
