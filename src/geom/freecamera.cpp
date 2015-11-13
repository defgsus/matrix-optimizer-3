/** @file freecamera.cpp

    @brief Free floating camera transform

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/3/2014</p>
*/

#include "freecamera.h"
#include "math/vector.h"

namespace MO {


FreeCamera::FreeCamera()
    : matrix_   (1.0)
    , inverse_  (false)
{
}

void FreeCamera::setMatrix(const Mat4 & m)
{
    matrix_ = m;
}

Vec3 FreeCamera::forward() const
{
    return Vec3(matrix_[0][2], matrix_[1][2], matrix_[2][2]);
}

void FreeCamera::moveX(Float steps)
{
    if (inverse_)
        matrix_ = glm::translate(matrix_, Vec3(steps,0,0));
    else
        matrix_ = glm::translate(Mat4(1.0), Vec3(steps,0,0)) * matrix_;
}

void FreeCamera::moveY(Float steps)
{
    if (inverse_)
        matrix_ = glm::translate(matrix_, Vec3(0,steps,0));
    else
        matrix_ = glm::translate(Mat4(1.0), Vec3(0,steps,0)) * matrix_;
}

void FreeCamera::moveZ(Float steps)
{
    if (inverse_)
        matrix_ = glm::translate(matrix_, Vec3(0,0,steps));
    else
        matrix_ = glm::translate(Mat4(1.0), Vec3(0,0,steps)) * matrix_;
}

void FreeCamera::move(const Vec3& steps)
{
    if (inverse_)
        matrix_ = glm::translate(matrix_, steps);
    else
        matrix_ = glm::translate(Mat4(1.0), steps) * matrix_;
}

void FreeCamera::rotateX(Float degree)
{
    if (inverse_)
        matrix_ = MATH::rotate(matrix_, -degree, Vec3(1,0,0));
    else
        matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(1,0,0)) * matrix_;
}

void FreeCamera::rotateY(Float degree)
{
    if (inverse_)
        matrix_ = MATH::rotate(matrix_, -degree, Vec3(0,1,0));
    else
        matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(0,1,0)) * matrix_;
}

void FreeCamera::rotateZ(Float degree)
{
    if (inverse_)
        matrix_ = MATH::rotate(matrix_, -degree, Vec3(0,0,1));
    else
        matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(0,0,1)) * matrix_;
}

void FreeCamera::setPosition(const Vec3 & v)
{
    matrix_[3][0] = v.x;
    matrix_[3][1] = v.y;
    matrix_[3][2] = v.z;
}




void FreeFloatCamera::applyVelocity(Float deltaVel, Float deltaRot)
{
    cam_.move(velo_ * deltaVel);
    cam_.rotateZ(veloRot_.z * deltaRot);
    cam_.rotateY(veloRot_.y * deltaRot);
    cam_.rotateX(veloRot_.x * deltaRot);
}

void FreeFloatCamera::applyDamping(Float delta)
{
    velo_ -= 0.2f * delta * velo_;
    veloRot_ -= 0.2f * delta * veloRot_;
}




} // namespace MO
