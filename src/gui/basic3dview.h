/** @file

    @brief prototype for a view into an opengl scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/23</p>
*/
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

    Mat4 transformationMatrix() const;

signals:

public slots:

    void viewInit(Float distanceZ = 10.f);
    void viewRotateX(Float degree);
    void viewRotateY(Float degree);

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

    Mat4
        projectionMatrix_,
        rotationMatrix_;
    Float distanceZ_;

    QPoint lastMousePos_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_BASIC3DVIEW_H
