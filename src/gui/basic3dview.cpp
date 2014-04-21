/***************************************************************************

Copyright (C) 2014  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#include <QPainter>
#include <QMouseEvent>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "basic3dview.h"

Basic3DView::Basic3DView(QWidget *parent) :
    QGLWidget(parent)
{
    viewInit();
}

void Basic3DView::viewInit(MO::Float distanceZ)
{
    distanceZ_ = distanceZ;
    rotationMatrix_ = MO::Mat4();
    updateGL();
}

void Basic3DView::viewRotateX(MO::Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(MO::Mat4(), d, MO::Vec3(1,0,0))
            * rotationMatrix_;
    updateGL();
}

void Basic3DView::viewRotateY(MO::Float d)
{
    if (!d) return;
    rotationMatrix_ =
            glm::rotate(MO::Mat4(), d, MO::Vec3(0,1,0))
            * rotationMatrix_;
    updateGL();
}

MO::Mat4 Basic3DView::transformationMatrix() const
{
    MO::Mat4 m = glm::translate(MO::Mat4(), MO::Vec3(0,0,-distanceZ_));
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
