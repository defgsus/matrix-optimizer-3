/** @file cameracontrolwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 29.04.2015</p>
*/

#include <QKeyEvent>
#include <QMouseEvent>

#include "cameracontrolwidget.h"
#include "geom/freecamera.h"
//#include "io/log.h"

namespace MO {
namespace GUI {


CameraControlWidget::CameraControlWidget(QWidget *parent)
    : QWidget           (parent)
    , p_cameraControl_  (new FreeCamera())
{
}

CameraControlWidget::~CameraControlWidget()
{
    delete p_cameraControl_;
}

const Mat4& CameraControlWidget::cameraMatrix() const
{
    return p_cameraControl_->getMatrix();
}

void CameraControlWidget::setCameraMatrix(const Mat4 & m)
{
    p_cameraControl_->setMatrix(m);
}

void CameraControlWidget::keyPressEvent(QKeyEvent * e)
{
    e->ignore();

    if (e->key() == Qt::Key_R)
    {
        p_cameraControl_->setMatrix(Mat4(1));
        emit cameraMatrixChanged(p_cameraControl_->getMatrix());
    }

    if (!e->isAccepted())
        QWidget::keyPressEvent(e);
}

void CameraControlWidget::mousePressEvent(QMouseEvent * e)
{
    //MO_DEBUG("down");

    p_lastMousePos_ = e->pos();

    QWidget::mousePressEvent(e);
}

void CameraControlWidget::mouseMoveEvent(QMouseEvent * e)
{
    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    int dx = p_lastMousePos_.x() - e->x(),
        dy = p_lastMousePos_.y() - e->y();
    p_lastMousePos_ = e->pos();

    bool send = false;

    if (e->buttons() & Qt::LeftButton)
    {
        p_cameraControl_->moveX(-0.03*fac*dx);
        p_cameraControl_->moveY( 0.03*fac*dy);

        send = true;
    }

    if (e->buttons() & Qt::RightButton)
    {
        p_cameraControl_->rotateX(-dy * fac);
        p_cameraControl_->rotateY(-dx * fac);

        send = true;
    }

    if (send)
    {
        emit cameraMatrixChanged(p_cameraControl_->getMatrix());
        e->accept();
    }
    else
    {
        QWidget::mouseMoveEvent(e);
    }
}

void CameraControlWidget::mouseReleaseEvent(QMouseEvent * e)
{
    QWidget::mouseReleaseEvent(e);
}

void CameraControlWidget::wheelEvent(QWheelEvent * e)
{
    Float fac = e->modifiers() & Qt::SHIFT ?
                10.f : e->modifiers() & Qt::CTRL? 0.025f : 1.f;

    Float d = std::max(-1, std::min(1, e->delta() ));
    p_cameraControl_->moveZ(-0.3 * d * fac);

    emit cameraMatrixChanged(p_cameraControl_->getMatrix());

    e->accept();
}


} // namespace GUI
} // namespace MO
