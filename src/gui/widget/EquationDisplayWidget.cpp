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

#include "EquationDisplayWidget.h"
#include "math/funcparser/parser.h"
#include "gui/painter/Grid.h"
#include "gui/painter/ValueCurve.h"
#include "io/log.h"
#include "math/functions.h"
#include "math/constants.h"

namespace MO {
namespace GUI {

class EquationDisplayWidget::EquationData : public PAINTER::ValueCurveData
{
    PPP_NAMESPACE::Parser * p;
    Double & x, & xr;
public:
    EquationData(PPP_NAMESPACE::Parser * p, Double & x, Double & xr)
        : p(p), x(x), xr(xr)
    { }
    Double value(Double time) const Q_DECL_OVERRIDE
    {
        x = time;
        xr = x * TWO_PI;
        return p->eval();
    }
};




EquationDisplayWidget::EquationDisplayWidget(QWidget *parent) :
    QWidget     (parent),
    parser_     (new PPP_NAMESPACE::Parser()),
    curveData_  (0),
    curve_      (0),
    grid_       (new PAINTER::Grid(this))
{
    parser_->variables().add("x", &varX_, tr("vertical position").toStdString());
    parser_->variables().add("y", &varY_, tr("horizontal position").toStdString());
    parser_->variables().add("xr", &varXR_, tr("vertical position in radians").toStdString());
    parser_->variables().add("yr", &varYR_, tr("horizontal position in radians").toStdString());

    setEquation("sin(xr) * cos(yr)");

    setPaintMode(PM_F_OF_X);

    viewSpace_.setX(-1);
    viewSpace_.setY(-1);
    viewSpace_.setScaleX(2);
    viewSpace_.setScaleY(2);

    for (int i=255; i>=0; --i)
        palette_.push_back(QColor(i, i*0.7, i*0.3));
    for (int i=0; i<256; ++i)
        palette_.push_back(QColor(i*0.4, i, i*0.7));

}

EquationDisplayWidget::~EquationDisplayWidget()
{
    delete parser_;
    delete curveData_;
}

void EquationDisplayWidget::resetViewSpace()
{
    Double aspect = Double(width()) / height();
    switch (mode_)
    {
        case PM_F_OF_X:
        case PM_F_OF_XY:
            setViewSpace(UTIL::ViewSpace(-1*aspect,-1,2*aspect,2));
            break;
        case PM_2D_INTEGER_NUM:
        case PM_2D_INTEGER_SQUARE:
            setViewSpace(UTIL::ViewSpace(-10*aspect,-10,20*aspect,20));
            break;
    }
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

void EquationDisplayWidget::setPaintMode(PaintMode m)
{
    mode_ = m;
    switch (mode_)
    {
        case PM_F_OF_X:
        case PM_F_OF_XY:
            grid_->setSpacingX(PAINTER::Grid::GS_SECONDS);
            grid_->setSpacingY(PAINTER::Grid::GS_SMALL_AND_LARGE);
            break;
        case PM_2D_INTEGER_NUM:
        case PM_2D_INTEGER_SQUARE:
            grid_->setSpacingX(PAINTER::Grid::GS_INTEGER);
            grid_->setSpacingY(PAINTER::Grid::GS_INTEGER);
            break;
    }
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
            scale = e->delta() > 0 ? 1.1 : 0.9;

    viewSpace_.zoom(scale, scale, tx, ty);

    emit viewSpaceChanged(viewSpace_);
    update();
}

int specialTrunc(double x)
{
    return x>=0? std::trunc(x) : (-std::trunc(-x-0.0000000123456)-1);
}

void EquationDisplayWidget::paintEvent(QPaintEvent * e)
{
    varY_ = 0;
    varX_ = 0;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    if (mode_ == PM_F_OF_X)
    {
        if (!curveData_)
            curveData_ = new EquationData(parser_, varX_, varXR_);
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

    if (mode_ == PM_2D_INTEGER_NUM || mode_ == PM_2D_INTEGER_SQUARE)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(0,0,0)));
        p.drawRect(rect());

        const Double
                bugfix = 9000000000,
                xo = MATH::moduloSigned(viewSpace_.mapXTo(0)+bugfix, 1.0),
                yo = MATH::moduloSigned(viewSpace_.mapYTo(0)+bugfix, 1.0),
                w = width() / std::max(1.0, viewSpace_.scaleX()),
                h = height() / std::max(1.0, viewSpace_.scaleY());
        const int
                numx = width() / w + 2,
                numy = height() / h + 2;

     //   MO_DEBUG("xo="<<xo<<" map0="<<viewSpace_.mapXTo(0));

        if (mode_ == PM_2D_INTEGER_SQUARE)
        {
            const int
                    w1 = std::max(1, (int)w-1),
                    h1 = std::max(1, (int)h-1);
            p.setPen(Qt::NoPen);
            for (int j=-1; j<numy; ++j)
            {
                const Double y = j - yo;
                const int iy = height() - 1 - (y+1) * h;
                for (int i=-1; i<numx; ++i)
                {
                    const Double x = i - xo;
                    const int ix = x * w;

                    varX_ = specialTrunc(i + viewSpace_.x());
                    varY_ = specialTrunc(j + viewSpace_.y());
                    varXR_ = varX_ * TWO_PI;
                    varYR_ = varY_ * TWO_PI;
                    Double v = parser_->eval();

                    const int vc = 255 + std::max(-255, std::min(255, (int)(v * 255)));
                    p.setBrush(QBrush(palette_[vc]));
                    p.drawRect(ix, iy, w1, h1);
                }
            }
        }
        if (mode_ == PM_2D_INTEGER_NUM)
        {
            p.setPen(QPen(QColor(255,255,255)));
            for (int j=-1; j<numy; ++j)
            {
                const Double y = j - yo;
                const int iy = height() - 1 - (y+1) * h;
                for (int i=-1; i<numx; ++i)
                {
                    const Double x = i - xo;
                    const int ix = x * w;

                    varX_ = specialTrunc(i + viewSpace_.x());
                    varY_ = specialTrunc(j + viewSpace_.y());
                    varXR_ = varX_ * TWO_PI;
                    varYR_ = varY_ * TWO_PI;
                    Double v = parser_->eval();

                    if (v != 0)
                    {
                        QRect r(ix,iy,w,h);
                        p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter,
                               QString::number(v), &r);
                    }
                }
            }
        }

        grid_->setViewSpace(viewSpace_);
        grid_->paint(p);

    }

    if (mode_ == PM_F_OF_XY)
    {
        const int w = 4, h = 4,
                numx = width()/w + 1,
                numy = height()/h + 1;

        p.setPen(Qt::NoPen);
        for (int y=0; y<numy; ++y)
        for (int x=0; x<numx; ++x)
        {
            varX_ = viewSpace_.mapXTo(Float(x*w)/width());
            varY_ = viewSpace_.mapYTo(Float(height()-1-y*h)/height());
            varXR_ = varX_ * TWO_PI;
            varYR_ = varY_ * TWO_PI;
            Double v = parser_->eval();

            const int vc = 255 + std::max(-255, std::min(255, (int)(v * 255)));
            p.setBrush(QBrush(palette_[vc]));
            p.drawRect(x*w, y*w, w, h);
        }

        grid_->setViewSpace(viewSpace_);
        grid_->paint(p);
    }
}


} // namespace GUI
} // namespace MO
