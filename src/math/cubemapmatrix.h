/** @file cubemapmatrix.h

    @brief predefined matrices for cubemap projection

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/2/2014</p>
*/

#ifndef MOSRC_MATH_CUBEMAPMATRIX_H
#define MOSRC_MATH_CUBEMAPMATRIX_H

#include "types/vector.h"

namespace MO {
namespace MATH {

class CubeMapMatrix
{
public:
    CubeMapMatrix();

    static const Mat4& matrix(uint i);

    const static Mat4 positiveX;
    const static Mat4 negativeX;
    const static Mat4 positiveY;
    const static Mat4 negativeY;
    const static Mat4 positiveZ;
    const static Mat4 negativeZ;
};

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_CUBEMAPMATRIX_H
