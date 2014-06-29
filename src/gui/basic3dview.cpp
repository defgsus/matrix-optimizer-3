/** @file

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/23</p>
*/
#include <QPainter>
#include <QMouseEvent>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "basic3dview.h"

namespace MO {
namespace GUI {

Basic3DView::Basic3DView(QWidget *parent) :
    QGLWidget(parent)
{
    viewInit();
}

void Basic3DView::viewInit(Float distanceZ)
{
    distanceZ_ = distanceZ;
    rotationMatrix_ = Mat4();
    updateGL();
}

void Basic3DView::viewRotateX(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(1,0,0))
            * rotationMatrix_;
    updateGL();
}

void Basic3DView::viewRotateY(Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(Mat4(), d, Vec3(0,1,0))
            * rotationMatrix_;
    updateGL();
}

Mat4 Basic3DView::transformationMatrix() const
{
    Mat4 m = glm::translate(Mat4(), Vec3(0,0,-distanceZ_));
    return m * rotationMatrix_;
}

void Basic3DView::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DView::mouseMoveEvent(QMouseEvent * e)
{
    int dx = lastMousePos_.x() - e->x(),
        dy = lastMousePos_.y() - e->y();
    lastMousePos_ = e->pos();

    if (e->buttons() & Qt::LeftButton)
    {
        viewRotateX(dy);
        viewRotateY(dx);
    }
}

void Basic3DView::resizeGL(int w, int h)
{
    projectionMatrix_ = glm::perspective(63.f, (float)w/h, 0.1f, 1000.0f);

    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix_));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void Basic3DView::paintGL()
{
    glLoadMatrixf(glm::value_ptr(transformationMatrix()));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    drawCoords_(10);
}


void Basic3DView::drawCoords_(int len)
{
    const GLfloat s = 0.1;
    glBegin(GL_LINES);

        glColor3f(1,0,0);
        for (int i=0; i<len; ++i)
        {
            glVertex3f(i,0,0);
            glVertex3f(i+1,0,0);
            glVertex3f(i+1,-s,0);
            glVertex3f(i+1,s,0);
        }
        glColor3f(0,1,0);
        for (int i=0; i<len; ++i)
        {
            glVertex3f(0,i,0);
            glVertex3f(0,i+1,0);
            glVertex3f(-s,i+1,0);
            glVertex3f(s,i+1,0);
        }
        glColor3f(0,0,1);
        for (int i=0; i<len; ++i)
        {
            glVertex3f(0,0,i);
            glVertex3f(0,0,i+1);
            glVertex3f(0,-s,i+1);
            glVertex3f(0,s,i+1);
        }

    glEnd();
}

} // namespace GUI
} // namespace MO
