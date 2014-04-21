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

#ifndef MOSRC_GUI_BASIC3DVIEWER_H
#define MOSRC_GUI_BASIC3DVIEWER_H

#include <QGLWidget>

#include "types/vector.h"

class Basic3DViewer : public QGLWidget
{
    Q_OBJECT
public:
    explicit Basic3DViewer(QWidget *parent = 0);

signals:

public slots:

    void viewInit(MO::Float distanceZ = 10.f);
    void viewRotateX(MO::Float degree);
    void viewRotateY(MO::Float degree);

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    //void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    /** Draws coordinate system */
    void drawCoords_(int len);

    MO::Mat4
        projectionMatrix_,
        transformationMatrix_;


    QPoint lastMousePos_;
};

#endif // MOSRC_GUI_BASIC3DVIEWER_H
