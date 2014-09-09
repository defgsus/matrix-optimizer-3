/** @file freecamera.cpp

    @brief Free floating camera transform

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/3/2014</p>
*/

#include "freecamera.h"
#include "math/vector.h"

namespace MO {
namespace GEOM {


FreeCamera::FreeCamera()
    : matrix_   (1.0)
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
    matrix_ = glm::translate(Mat4(1.0), Vec3(steps,0,0)) * matrix_;
}

void FreeCamera::moveY(Float steps)
{
    matrix_ = glm::translate(Mat4(1.0), Vec3(0,steps,0)) * matrix_;
}

void FreeCamera::moveZ(Float steps)
{
    matrix_ = glm::translate(Mat4(1.0), Vec3(0,0,steps)) * matrix_;
}

void FreeCamera::rotateX(Float degree)
{
    matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(1,0,0)) * matrix_;
}

void FreeCamera::rotateY(Float degree)
{
    matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(0,1,0)) * matrix_;
}

void FreeCamera::rotateZ(Float degree)
{
    matrix_ = MATH::rotate(Mat4(1.0), degree, Vec3(0,0,1)) * matrix_;
}

void FreeCamera::moveTo(const Vec3 & v)
{
    matrix_[3][0] = v.x;
    matrix_[3][1] = v.y;
    matrix_[3][2] = v.z;
}

} // namespace GEOM
} // namespace MO
