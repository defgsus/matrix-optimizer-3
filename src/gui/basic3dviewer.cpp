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
#include <QDebug>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "basic3dviewer.h"

Basic3DViewer::Basic3DViewer(QWidget *parent) :
    QGLWidget(parent)
{
    viewInit();
}

void Basic3DViewer::viewInit(MO::Float distanceZ)
{
    transformationMatrix_ = glm::translate(MO::Mat4(), MO::Vec3(0,0,-distanceZ));
    updateGL();
}

void Basic3DViewer::viewRotateX(MO::Float d)
{
    if (!d) return;
    transformationMatrix_ *=
            glm::rotate(MO::Mat4(), d, MO::Vec3(1,0,0));
    updateGL();
}

void Basic3DViewer::viewRotateY(MO::Float d)
{
    if (!d) return;
    transformationMatrix_ *=
        glm::rotate(MO::Mat4(), d, MO::Vec3(0,1,0));
    updateGL();
}


void Basic3DViewer::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void Basic3DViewer::mouseMoveEvent(QMouseEvent * e)
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

void Basic3DViewer::resizeGL(int w, int h)
{
    qDebug() << w << h;

    projectionMatrix_ = glm::perspective(63.f, (float)w/h, 0.1f, 1000.0f);

    glViewport(0,0,w,h);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projectionMatrix_));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void Basic3DViewer::paintGL()
{
    glLoadMatrixf(glm::value_ptr(transformationMatrix_));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawCoords_(10);
}


void Basic3DViewer::drawCoords_(int len)
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
