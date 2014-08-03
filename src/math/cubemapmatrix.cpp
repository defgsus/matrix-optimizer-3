/** @file cubemapmatrix.cpp

    @brief predefined matrices for cubemap projection

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/2/2014</p>
*/

#include "cubemapmatrix.h"

namespace MO {
namespace MATH {


CubeMapMatrix::CubeMapMatrix()
{
}

const Mat4& CubeMapMatrix::matrix(uint i)
{
    switch (i)
    {
    case 0:  return positiveX;
    case 1:  return negativeX;
    case 2:  return positiveY;
    case 3:  return negativeY;
    case 4:  return positiveZ;
    default: return negativeZ;
    }
}

const Mat4 CubeMapMatrix::identity = Mat4(1.0);

const Mat4 CubeMapMatrix::positiveX = Mat4(0,0,-1,0, 0,-1,0,0, -1,0,0,0, 0,0,0,1);

const Mat4 CubeMapMatrix::negativeX = Mat4(0,0,1,0, 0,-1,0,0, 1,0,0,0, 0,0,0,1);

const Mat4 CubeMapMatrix::positiveY = Mat4(1,0,0,0, 0,0,-1,0, 0,1,0,0, 0,0,0,1);

const Mat4 CubeMapMatrix::negativeY = Mat4(1,0,0,0, 0,0,1,0, 0,-1,0,0, 0,0,0,1);

const Mat4 CubeMapMatrix::positiveZ = Mat4(1,0,0,0, 0,-1,0,0, 0,0,-1,0, 0,0,0,1);

const Mat4 CubeMapMatrix::negativeZ = Mat4(-1,0,0,0, 0,-1,0,0, 0,0,1,0, 0,0,0,1);


} // namespace MATH
} // namespace MO
