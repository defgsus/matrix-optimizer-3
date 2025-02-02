/** @file cameracontrolwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 29.04.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_CAMERACONTROLWIDGET_H
#define MOSRC_GUI_WIDGET_CAMERACONTROLWIDGET_H

#include <QWidget>

#include "types/vector.h"

namespace MO {
class FreeCamera;
namespace GUI {

/** Widget with a FreeCamera and assigned mouse control */
class CameraControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraControlWidget(QWidget *parent = 0);
    ~CameraControlWidget();

    const Mat4& cameraMatrix() const;
    FreeCamera * cameraControl() { return p_cameraControl_; }
    const FreeCamera * cameraControl() const { return p_cameraControl_; }

signals:

    /** Send when the camera matrix changed (in free-camera-mode) */
    void cameraMatrixChanged(const MO::Mat4&);

public slots:

    void setCameraMatrix(const Mat4&);

protected:

    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);

private:

    FreeCamera * p_cameraControl_;
    QPoint p_lastMousePos_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CAMERACONTROLWIDGET_H
