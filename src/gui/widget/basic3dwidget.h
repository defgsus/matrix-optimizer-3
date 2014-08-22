/** @file basic3dwidget.h

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_BASIC3DWIDGET_H
#define MOSRC_GUI_WIDGET_BASIC3DWIDGET_H

// to include glew before gl.h
#include "gl/opengl.h"

#include <QGLWidget>

#include "types/vector.h"

namespace MO {
namespace GEOM { class FreeCamera; }
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
        RM_FRAMEBUFFER,
        RM_FULLDOME_CUBE
    };

    explicit Basic3DWidget(RenderMode mode, QWidget *parent = 0);
    ~Basic3DWidget();

    RenderMode renderMode() const { return renderMode_; }

    bool isGlInitialized() const { return isGlInitialized_; }

    const Mat4& projectionMatrix() const { return projectionMatrix_; }
    Mat4 viewMatrix() const;

signals:

    void glInitialized();
    void glReleased();

public slots:

    /** Call this to properly release openGL resources and
        wait for glReleased() signal before destruction. */
    void shutDownGL();

    void setRenderMode(RenderMode);

    void viewInit(Float distanceZ = 10.f);
    void viewRotateX(Float degree);
    void viewRotateY(Float degree);

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

    RenderMode renderMode_, nextRenderMode_;

    bool isGlInitialized_,
         closeRequest_,
         modeChangeRequest_,
         useFreeCamera_;

    Mat4
        projectionMatrix_,
        rotationMatrix_;
    Float distanceZ_;

    QPoint lastMousePos_;

    GEOM::FreeCamera * camera_;
    GL::FrameBufferObject * fbo_;
    GL::ScreenQuad * screenQuad_;
    GL::Drawable * gridObject_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_BASIC3DWIDGET_H
