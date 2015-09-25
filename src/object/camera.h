/** @file camera.h

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_CAMERA_H
#define MOSRC_OBJECT_CAMERA_H

#include "objectgl.h"
#include "object/interface/valuetextureinterface.h"

namespace MO {

class Camera : public ObjectGl, public ValueTextureInterface
{
    Q_OBJECT
public:
    enum RenderMode
    {
        RM_ORTHOGRAPHIC,
        RM_PERSPECTIVE,
        RM_FULLDOME_CUBE,
        RM_PROJECTOR_SLICE
    };


    MO_OBJECT_CONSTRUCTOR(Camera);
    ~Camera();

    virtual Type type() const Q_DECL_OVERRIDE { return T_CAMERA; }
    virtual bool isCamera() const Q_DECL_OVERRIDE { return true; }

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings&, uint, Double) Q_DECL_OVERRIDE { };

    // ---------- camera specific stuff -----------

    /** Returns the mode of rendering */
    RenderMode renderMode() const { return renderMode_; }

    /** Returns the list of objects that should be rendered according
        to wildcard parameters. */
    QList<ObjectGl*> getRenderObjects();

    /** Returns true when the wildcard parameters have changed and
        getRenderObjects() should be called again. */
    bool needsUpdateRenderObjects() const { return wildcardChanged_; }

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

    /** Returns the additional view transformation for the camera.
        Returns an identity matrix except for RM_FULLDOME_CUBE and RM_PROJECTOR_SLICE.
        In RM_FULLDOME_CUBE, @p index is the index [0,5] for each cubemap texture */
    const Mat4& cameraViewMatrix(uint index) const;

    /** Draws the contents of the framebuffer on a [-1,1] quad. */
    void drawFramebuffer(uint thread, Double time);

    /** Returns the framebuffer on which the camera renders, or NULL */
    GL::FrameBufferObject * fbo() const;

    /** Texture output interface */
    const GL::Texture * valueTexture(uint channel, Double time, uint thread) const Q_DECL_OVERRIDE;

    void setOverrideMatrix(const Mat4& m);
    void clearOverrideMatrix() { useOverrideMatrix_ = false; }

    void calculateTransformation(Mat4& matrix, Double time, uint thread) const
                                                                    Q_DECL_OVERRIDE;

signals:

public slots:

private:

    void updateFboSize_();

    GL::FrameBufferObject* fbo_;

    GL::ScreenQuad* screenQuad_;

    ParameterFloat * p_cameraMix_,
        *p_cameraAngle_, *p_cameraFdAngle_,
        *p_cameraOrthoScale_, *p_cameraOrthoMix_,
        *p_backR_, *p_backG_, *p_backB_, *p_backA_,
        *p_near_, *p_far_;
    ParameterSelect * p_cameraMode_, *p_magInterpol_, *p_enableOut_,
                    * p_multiSample_;
    ParameterInt * p_width_, * p_height_,
                 * p_cubeRes_;
    ParameterText * p_wcInclude_,
                  * p_wcIgnore_;
    GL::Uniform * uColor_, * uAngle_;

    Float aspectRatio_;
    RenderMode renderMode_;
    AlphaBlendSetting alphaBlend_;

    Mat4 sliceMatrix_, overrideMatrix_;
    bool useOverrideMatrix_, wildcardChanged_;
    mutable std::vector<Mat4> cheat_;
    Double overrideMatrixTime_;

    GL::Texture * blendTexture_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CAMERA_H
