/** @file equationdisplaywidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/1/2014</p>
*/

#include <QPainter>
#include <QPaintEvent>

#include "equationdisplaywidget.h"
#include "math/funcparser/parser.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"

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


void EquationDisplayWidget::setViewSpace(const UTIL::ViewSpace & vs)
{
    viewSpace_ = vs;
    update();
}


EquationDisplayWidget::EquationDisplayWidget(QWidget *parent) :
    QWidget     (parent),
    parser_     (new PPP_NAMESPACE::Parser()),
    curveData_  (0),
    curve_      (0)
{
    parser_->variables().add("x", &varX_);
    parser_->variables().add("y", &varY_);

    parser_->parse("sin(x*TWO_PI)");
}

EquationDisplayWidget::~EquationDisplayWidget()
{
    delete parser_;
    delete curveData_;
}

void EquationDisplayWidget::mousePressEvent(QMouseEvent * e)
{
    lastMousePos_ = e->pos();
}

void EquationDisplayWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        Float dx = viewSpace_.mapXDistanceTo(e->x() - lastMousePos_.x()),
              dy = viewSpace_.mapYDistanceTo(e->y() - lastMousePos_.y());

        viewSpace_.addX(dx);
        viewSpace_.addY(dy);

        update();
    }
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

        curve_->paint(p, e->rect());
    }
}


} // namespace GUI
} // namespace MO
