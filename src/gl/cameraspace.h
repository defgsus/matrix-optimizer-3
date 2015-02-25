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
        : width_(0), height_(0),
          fov_(0), proj_(1.0), cv_(1.0), v_(1.0), isCubemap_(false)
    { }

    // ------ getter ------------

    /** Framebuffer width */
    uint width() const { return width_; }
    /** Framebuffer height */
    uint height() const { return height_; }
    /** Returns the field of view in degree */
    Float fieldOfView() const { return fov_; }

    /** The projection matrix. Typically frustum or orthographic */
    const Mat4& projectionMatrix() const { return proj_; }
    /** The to-eye-space matrix (cubeview * view) */
    const Mat4& cubeViewMatrix() const { return cv_; }
    /** The to-eye-space matrix (view) */
    const Mat4& viewMatrix() const { return v_; }
    /** The center of camera position in world coords */
    const Vec3& position() const { return pos_; }
    /** Return, if cubemap rendering is active right now. */
    bool isCubemap() const { return isCubemap_; }

    // ------- setter -----------

    void setSize(uint width, uint height) { width_ = width; height_ = height; }
    void setFieldOfView(Float fov) { fov_ = fov; }

    void setProjectionMatrix(const Mat4& m) { proj_ = m; }
    void setCubeViewMatrix(const Mat4& m) { cv_ = m; }
    void setViewMatrix(const Mat4& m) { v_ = m; }
    void setIsCubemap(bool is) { isCubemap_ = is; }
    void setPosition(const Vec3& p) { pos_ = p; }

private:

    uint width_, height_;
    Float fov_;
    Mat4 proj_, cv_, v_;
    Vec3 pos_;
    bool isCubemap_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_CAMERASPACE_H
