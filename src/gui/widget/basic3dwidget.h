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
namespace GUI {

/** Override drawGL() to draw your stuff! */
class Basic3DWidget : public QGLWidget
{
    Q_OBJECT
public:

    enum RenderMode
    {
        RM_DIRECT,
        RM_FRAMEBUFFER,
        RM_FULLDOME_CUBE
    };

    explicit Basic3DWidget(RenderMode mode, QWidget *parent = 0);

    RenderMode renderMode() const { return renderMode_; }

    const Mat4& projectionMatrix() const { return projectionMatrix_; }
    Mat4 transformationMatrix() const;

signals:

    void glInitialized();

public slots:

    void viewInit(Float distanceZ = 10.f);
    void viewRotateX(Float degree);
    void viewRotateY(Float degree);

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void initializeGL() Q_DECL_OVERRIDE;

    /** Sets the viewport and the projection matrix */
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void paintGL() Q_DECL_OVERRIDE Q_DECL_FINAL;

    virtual void drawGL(const Mat4& projection, const Mat4& transformation) = 0;

private:

    RenderMode renderMode_;

    Mat4
        projectionMatrix_,
        rotationMatrix_;
    Float distanceZ_;

    QPoint lastMousePos_;

    GL::FrameBufferObject * fbo_;
    GL::ScreenQuad * screenQuad_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_BASIC3DWIDGET_H
