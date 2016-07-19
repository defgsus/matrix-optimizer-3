/** @file basic3dwidget.h

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_BASIC3DWIDGET_H
#define MOSRC_GUI_WIDGET_BASIC3DWIDGET_H

// to include glbinding before Qt's include of gl.h
#include "gl/opengl.h"

#include <QGLWidget>
#include <QSize>

#include "gl/opengl_undef.h"
#include "types/vector.h"

namespace MO {
class FreeCamera;
namespace GUI {

/** A super-duper opengl shader based, fulldome-cube-able draw area.

    Override drawGL() to draw your stuff!
    Override initGL() and releaseGL() to aquire and release resources.

    Important: Don't just destroy the widget.
    Call shutDownGL() or close() first and wait for the glReleased() signal!

    */
class Basic3DWidget : public QGLWidget
{
    Q_OBJECT
public:

    enum RenderMode
    {
        RM_DIRECT,
        RM_DIRECT_ORTHO,
        RM_FRAMEBUFFER_ORTHO,
        RM_FRAMEBUFFER,
        RM_FULLDOME_CUBE
    };

    enum ViewDirection
    {
        VD_FRONT,
        VD_BACK,
        VD_TOP,
        VD_BOTTOM,
        VD_LEFT,
        VD_RIGHT
    };

    enum CameraMode
    {
        /** Camera always faces the origin */
        CM_CENTER,
        /** Camera can be freely adjusted */
        CM_FREE,
        CM_FREE_INVERSE,
        /** Camera is defined by setProjectionMatrix() and setViewMatrix() */
        CM_SET
    };

    explicit Basic3DWidget(RenderMode mode, QWidget *parent = 0);
    ~Basic3DWidget();

    const QSize& fboSize() const { return fboSize_; }

    RenderMode renderMode() const { return renderMode_; }
    CameraMode cameraMode() const { return cameraMode_; }

    bool isGlInitialized() const { return isGlInitialized_; }

    const Mat4& projectionMatrix() const;
    Mat4 viewMatrix() const;

signals:

    void glInitialized();
    void glReleased();

public slots:

    /** Call this to properly release openGL resources and
        wait for glReleased() signal before destruction. */
    void shutDownGL();

    /** Sets the resolution of the framebuffer.
        In RM_FULLDOME_CUBE, each cube texture will have this resolution.
        In DIRECT modes, the setting is ignored. */
    void setFboSize(const QSize&);
    void setFboSize(uint w, uint h) { setFboSize(QSize(w,h)); }

    void setRenderMode(RenderMode);
    void setCameraMode(CameraMode);

    /** Sets the camera matrix to one of the ViewDirection enums,
        with distance to center as @p distance */
    void viewSet(ViewDirection dir, Float distance);

    /** Rotates the current camera */
    void viewRotateX(Float degree);

    /** Rotates the current camera */
    void viewRotateY(Float degree);

    /** Sets the (horizontal) scale of the orthographic projection.
        The view extends in negative and positive units @p s. */
    void viewSetOrthoScale(Float s) { orthoScaleX_ = orthoScaleY_ = s; updateProjection_(); update(); }

    /** Sets the (horizontal) scale of the orthographic projection.
        The view extends in negative and positive units @p s. */
    void viewSetOrthoScale(Float sx, Float sy) { orthoScaleX_ = sx; orthoScaleY_ = sy; updateProjection_(); update(); }

    /** Sets the projection matrix when in CM_SET mode */
    void setProjectionMatrix(const Mat4&);

    /** Sets the view matrix when in CM_SET mode */
    virtual void setViewMatrix(const Mat4&);

protected:

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void closeEvent(QCloseEvent *);

    /** Override this to create resources-per-opengl-context. */
    virtual void initGL() { }

    /** Override to release opengl resources. */
    virtual void releaseGL() { }

    /** Sets the viewport and the projection matrix */
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    /** This is used by Basic3DWidget only.
        Override drawGL() to implement drawing. */
    void paintGL() Q_DECL_OVERRIDE Q_DECL_FINAL;

    /** This is called before drawGL() and before any Basic3DWidget framebuffers are bound.
        Usefull to lazy-initialize gl resources. */
    virtual void prepareDrawGL() { };

    /** Override to draw your stuff */
    virtual void drawGL(const Mat4& projection,
                        const Mat4& cubeViewTrans,
                        const Mat4& viewtrans,
                        const Mat4& trans) = 0;

    /** Can be called by derived classes in their drawGL() routine. */
    virtual void drawGrid(const Mat4& projection,
                          const Mat4& cubeViewTrans,
                          const Mat4& viewTrans,
                          const Mat4& trans);

private:

    /** Don't touch this: use initGL() instead */
    void initializeGL() Q_DECL_OVERRIDE Q_DECL_FINAL;

    void createGLStuff_();
    void releaseGLStuff_();
    void releaseGL_();
    void updateProjection_();

    RenderMode renderMode_, nextRenderMode_;
    CameraMode cameraMode_;

    bool isGlInitialized_,
         closeRequest_,
         modeChangeRequest_;

    Mat4
        projectionMatrix_,
        fixProjectionMatrix_,
        fixViewMatrix_,
        rotationMatrix_;
    Float distanceZ_,
        orthoScaleX_,
        orthoScaleY_;

    QPoint lastMousePos_;

    QSize fboSize_;

    FreeCamera * camera_;
    GL::FrameBufferObject * fbo_;
    GL::ScreenQuad * screenQuad_;
    GL::Drawable * gridObject_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_BASIC3DWIDGET_H
