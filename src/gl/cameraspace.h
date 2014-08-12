/** @file CameraSpace.h

    @brief Camera matrix container

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_CAMERASPACE_H
#define MOSRC_GL_CAMERASPACE_H

#include <types/vector.h>
#include "lightsettings.h"

namespace MO {
namespace GL {

class CameraSpace
{
public:
    // XXX LightSettings? - move CameraSpace and LightSettings to a RenderSettings class
    CameraSpace(const LightSettings& lights)
        :   proj_(1.0), cv_(1.0), v_(1.0), lights_(lights)
    { }

    // ------ getter ------------

    /** The projection matrix. Typically rustum or orthographic */
    const Mat4& projectionMatrix() const { return proj_; }
    /** The to-eye-space matrix (cubeview * view) */
    const Mat4& cubeViewMatrix() const { return cv_; }
    /** The to-eye-space matrix (view) */
    const Mat4& viewMatrix() const { return v_; }
    /** The lighting settings */
    const LightSettings& lights() const { return lights_; }

    // ------- setter -----------

    void setProjectionMatrix(const Mat4& m) { proj_ = m; }
    void setCubeViewMatrix(const Mat4& m) { cv_ = m; }
    void setViewMatrix(const Mat4& m) { v_ = m; }

private:

    Mat4 proj_, cv_, v_;
    const LightSettings & lights_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CAMERASPACE_H
