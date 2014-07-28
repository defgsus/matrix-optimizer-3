/** @file basic3dwidget.cpp

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QPainter>
#include <QMouseEvent>

#include "basic3dwidget.h"

namespace MO {
namespace GUI {

Basic3DWidget::Basic3DWidget(QWidget *parent) :
    QGLWidget(parent)
{
    QGLFormat f(format());
    f.setDepth(true);
    setFormat(f);

    viewInit();
}

void Basic3DWidget::viewInit(Float distanceZ)
{
    distanceZ_ = distanceZ;
    rotationMatrix_ = Mat4();
    updateGL();
}

void Basic3DWidget::viewRotateX(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(1,0,0))
            * rotationMatrix_;
    updateGL();
}

void Basic3DWidget::viewRotateY(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(0,1,0))
            * rotationMatrix_;
    updateGL();
}

Mat4 Basic3DWidget::transformationMatrix() const
{
    Mat4 m = glm::translate(Mat4(), Vec3(0,0,-distanceZ_));
    return m * rotationMatrix_;
}

void Basic3DWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DWidget::mouseMoveEvent(QMouseEvent * e)
{
    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    if (e->buttons() & Qt::LeftButton)
    {
        viewRotateX(dy);
        viewRotateY(dx);
    }

    if (e->buttons() & Qt::RightButton)
    {
        distanceZ_ -= 0.05 * dy;
        updateGL();
    }
}


void Basic3DWidget::initializeGL()
{
    MO_CHECK_GL( glEnable(GL_DEPTH_TEST) );

    emit glInitialized();
}

void Basic3DWidget::resizeGL(int w, int h)
{
    projectionMatrix_ = glm::perspective(63.f, (float)w/h, 0.1f, 1000.0f);

    MO_CHECK_GL( glViewport(0,0,w,h) );
}


} // namespace GUI
} // namespace MO
