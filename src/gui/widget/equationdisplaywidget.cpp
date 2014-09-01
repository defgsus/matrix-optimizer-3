/** @file equationdisplaywidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "equationdisplaywidget.h"
#include "math/funcparser/parser.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"
#include "io/log.h"

namespace MO {
namespace GUI {

class EquationDisplayWidget::EquationData : public PAINTER::ValueCurveData
{
    PPP_NAMESPACE::Parser * p;
    Double & x;
public:
    EquationData(PPP_NAMESPACE::Parser * p, Double & x)
        : p(p), x(x)
    { }
    Double value(Double time) const Q_DECL_OVERRIDE
    {
        x = time;
        return p->eval();
    }
};




EquationDisplayWidget::EquationDisplayWidget(QWidget *parent) :
    QWidget     (parent),
    parser_     (new PPP_NAMESPACE::Parser()),
    curveData_  (0),
    curve_      (0),
    grid_       (0)
{
    parser_->variables().add("x", &varX_);
    parser_->variables().add("y", &varY_);

    setEquation("sin(x*TWO_PI)");

    viewSpace_.setX(-1);
    viewSpace_.setY(-1);
    viewSpace_.setScaleX(2);
    viewSpace_.setScaleY(2);
}

EquationDisplayWidget::~EquationDisplayWidget()
{
    delete parser_;
    delete curveData_;
}

void EquationDisplayWidget::setViewSpace(const UTIL::ViewSpace & vs)
{
    viewSpace_ = vs;
    update();
}

void EquationDisplayWidget::setEquation(const QString &equation)
{
    equation_ = equation;
    parser_->parse(equation_.toStdString());
    update();
}



void EquationDisplayWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
    lastViewSpace_ = viewSpace_;
}

void EquationDisplayWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        Float   dx = lastViewSpace_.mapXDistanceTo(
                    Float(e->x() - lastMousePos_.x())/width()),
                dy = lastViewSpace_.mapYDistanceTo(
                    Float(e->y() - lastMousePos_.y())/height());

        viewSpace_.setX(lastViewSpace_.x() - dx);
        viewSpace_.setY(lastViewSpace_.y() + dy);

        emit viewSpaceChanged(viewSpace_);
        update();
    }
}

void EquationDisplayWidget::wheelEvent(QWheelEvent * e)
{
    const Float
            tx = Float(e->x()) / width(),
            ty = Float(1) - Float(e->y()) / height(),
            scale = e->delta() < 0 ? 1.1 : 0.9;

    viewSpace_.zoom(scale, scale, tx, ty);

    emit viewSpaceChanged(viewSpace_);
    update();
}

void EquationDisplayWidget::paintEvent(QPaintEvent * e)
{
    varY_ = 0;
    varX_ = 0;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // f(x)
    if (1)
    {
        if (!grid_)
            grid_ = new PAINTER::Grid(this);

        if (!curveData_)
            curveData_ = new EquationData(parser_, varX_);
        if (!curve_)
        {
            curve_ = new PAINTER::ValueCurve(this);
            curve_->setCurveData(curveData_);
        }

        p.setBrush(Qt::black);
        p.setPen(Qt::NoPen);
        p.drawRect(e->rect());

        grid_->setViewSpace(viewSpace_);
        grid_->paint(p, e->rect());

        curve_->setViewSpace(viewSpace_);
        curve_->paint(p, e->rect());
    }
}


} // namespace GUI
} // namespace MO
