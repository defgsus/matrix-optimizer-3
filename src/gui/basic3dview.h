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

#ifndef MOSRC_GUI_BASIC3DVIEW_H
#define MOSRC_GUI_BASIC3DVIEW_H

#include <QGLWidget>

#include "types/vector.h"

namespace MO {
namespace GUI {

class Basic3DView : public QGLWidget
{
    Q_OBJECT
public:
    explicit Basic3DView(QWidget *parent = 0);

    MO::Mat4 transformationMatrix() const;

signals:

public slots:

    void viewInit(MO::Float distanceZ = 10.f);
    void viewRotateX(MO::Float degree);
    void viewRotateY(MO::Float degree);

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    //void initializeGL();

    /** Sets the viewport and the projection matrix */
    void resizeGL(int w, int h);
    /** Clears buffer and applies the transformation matrix */
    void paintGL();

    /** Draws coordinate system */
    void drawCoords_(int len);

private:

    MO::Mat4
        projectionMatrix_,
        rotationMatrix_;
    MO::Float distanceZ_;

    QPoint lastMousePos_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_BASIC3DVIEW_H
