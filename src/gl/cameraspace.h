/** @file CameraSpace.h

    @brief Camera matrix container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_CAMERASPACE_H
#define MOSRC_GL_CAMERASPACE_H

#include <types/vector.h>

namespace MO {
namespace GL {

class CameraSpace
{
public:
    CameraSpace()
        :   proj_(1.0), view_(1.0)
    { }

    // ------ getter ------------

    const Mat4& projectionMatrix() const { return proj_; }
    const Mat4& viewMatrix() const { return view_; }

    // ------- setter -----------

    void setProjectionMatrix(const Mat4& m) { proj_ = m; }
    void setViewMatrix(const Mat4& m) { view_ = m; }

private:

    Mat4 proj_, view_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CAMERASPACE_H
