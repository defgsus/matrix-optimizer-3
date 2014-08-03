/** @file camera.h

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_CAMERA_H
#define MOSRC_OBJECT_CAMERA_H

#include "objectgl.h"

namespace MO {

class Camera : public ObjectGl
{
    Q_OBJECT
public:
    enum RenderMode
    {
        RM_PERSP,
        RM_FULLDOME_CUBE
    };


    MO_OBJECT_CONSTRUCTOR(Camera);
    ~Camera();

    virtual Type type() const Q_DECL_OVERRIDE { return T_CAMERA; }
    virtual bool isCamera() const Q_DECL_OVERRIDE { return true; }

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;
    virtual void setBufferSize(uint bufferSize, uint thread) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::CameraSpace&, uint, Double) Q_DECL_OVERRIDE { };

    // ---------- camera specific stuff -----------

    /** Returns the mode of rendering */
    RenderMode renderMode() const { return renderMode_; }

    //GL::FrameBufferObject * getFrameBuffer(uint thread) const { return fbo_[thread]; }

    /** Returns number of cubemap textures needed. */
    uint numCubeTextures(uint thread, Double time) const;

    /** Starts rendering an openGl frame for this camera.
        Binds the framebuffer and clears it. */
    void startGlFrame(uint thread, Double time, uint cubeMapIndex = 0);

    /** Finishes rendering an openGl frame for this camera.
        Only unbinds the framebuffer right now. */
    void finishGlFrame(uint thread, Double time);

    /** Initialize camera space with projection matrix */
    void initCameraSpace(GL::CameraSpace& cam, uint thread, Double time) const;

    /** Returns the view transformation for each cubemap texture */
    const Mat4& cubeMapMatrix(uint index) const;

    /** Draws the contents of the framebuffer on a [-1,1] quad. */
    void drawFramebuffer(uint thread, Double time);

signals:

public slots:

private:

    //std::vector<std::vector<Mat4>> projection_;
    std::vector<GL::FrameBufferObject*> fbo_;

    std::vector<GL::ScreenQuad*> screenQuad_;

    ParameterFloat * cameraMix_, *cameraAngle_;
    ParameterSelect * cameraMode_;
    GL::Uniform * uColor_, * uAngle_;

    Float aspectRatio_;
    RenderMode renderMode_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CAMERA_H
