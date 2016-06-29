/** @file

    @brief widget for MATH::Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/24</p>
*/


#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <QByteArray>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>

#include "timeline1dview.h"
#include "painter/grid.h"
#include "painter/valuecurve.h"
#include "painter/sequenceoverpaint.h"
#include "io/error.h"
#include "io/datastream.h"
#include "tool/enumnames.h"

#include <cmath>

namespace MO {
namespace GUI {

namespace {

class TimelineCurveData : public PAINTER::ValueCurveData
{
public:
    MATH::Timeline1d * timeline;
    virtual Double value(Double time) const { return timeline->get(time); }
};

}


Timeline1DView::Timeline1DView(MATH::Timeline1d * tl, QWidget *parent)
    :   QWidget                 (parent),
        tl_                     (tl),
        gridPainter_            (new PAINTER::Grid(this)),
        valuePainter_           (new PAINTER::ValueCurve(this)),
        valuePainterData_       (new TimelineCurveData),
        sequenceCurvePainter_   (0),
        sequenceOverpaint_      (0),

        options_                (O_EnableAll),
        handleRadius_           (5),
        handleRadiusHovered_    (handleRadius_*1.2),
        handleRadiusSelected_   (handleRadiusHovered_*1.4),
        zoomChange_             (0.05),
        modifierSelectFrame_    (Qt::SHIFT),
        modifierMultiSelect_    (Qt::CTRL),
        modifierMoveVert_       (Qt::CTRL),

        hoverHash_              (MATH::Timeline1d::InvalidHash),
        moveHoverHash_          (MATH::Timeline1d::InvalidHash),
        hoverCurveHash_         (MATH::Timeline1d::InvalidHash),
        derivativeHoverIndex_   (-1),
        action_                 (A_NOTHING)
{
    space_.setScaleX(10);

    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    valuePainter_->setCurveData(valuePainterData_);
}

Timeline1DView::~Timeline1DView()
{
    delete valuePainterData_;
}

void Timeline1DView::setStatusTip_(const QString & tip)
{
    setStatusTip(tip);
    emit statusTipChanged(tip);
}

void Timeline1DView::setTimeline(MATH::Timeline1d *timeline)
{
    tl_ = timeline;

    update();
}

void Timeline1DView::setGridOptions(int options)
{
    gridPainter_->setOptions(options);
    update();
}


Double Timeline1DView::screen2time(Double x) const
{
    return space_.mapXTo(x / width());
}

Double Timeline1DView::screen2value(Double y) const
{
    return space_.mapYTo( (height() - 1 - y) / height() );
}

int Timeline1DView::time2screen(Double time) const
{
    return space_.mapXFrom(time) * width();
}

int Timeline1DView::value2screen(Double val) const
{
    return height() - 1 - space_.mapYFrom(val) * height();
}

void Timeline1DView::changeScale_(int scrx, int scry, Double fx, Double fy)
{
    if (!(options_ & O_ZoomViewX))
        fx = 1;
    if (!(options_ & O_ZoomViewY))
        fy = 1;

    if ((fx!=1) || (fy!=1))
    {
        const Double
            tx = (Double)scrx / width(),
            ty = 1.0 - (Double)scry / height();

        space_.zoom(fx, fy, tx, ty);

        emit viewSpaceChanged(space_);
    }
}

void Timeline1DView::setOptions(int options)
{
    bool changed = (options != options_);
    options_ = options;

    if (changed)
        update();
}

void Timeline1DView::setViewSpace(const UTIL::ViewSpace &v, bool send)
{
    space_ = ospace_ = v;
    if (send)
        emit viewSpaceChanged(space_);
    update();
}

void Timeline1DView::setViewSpace(const UTIL::ViewSpace &timelineSpace,
                                  const UTIL::ViewSpace &overpainterSpace, bool send)
{
    space_ = timelineSpace;
    ospace_ = overpainterSpace;
    if (send)
        emit viewSpaceChanged(space_);
    update();
}

void Timeline1DView::fitToView(bool fitX, bool fitY, int marginInPixels)
{
    if (!tl_) return;
    fitToView_(tl_->tmin(), tl_->tmax(), fitX, fitY, marginInPixels);
}

void Timeline1DView::fitSelectionToView(bool fitX, bool fitY, int marginInPixels)
{
    if (!tl_ || !isSelected()) return;

    // find min/max time of selection
    Double tmin = 0.0, tmax = 0.0;

    bool f = true;
    for (auto h : selectHashSet_)
    {
        auto pointIt = tl_->getData().lower_bound(h);
        if (pointIt != tl_->getData().end())
        {
            if (f)
            {
                tmin = tmax = pointIt->second.t;
                f = false;
            }
            else
            {
                tmin = std::min(tmin, pointIt->second.t);
                tmax = std::max(tmax, pointIt->second.t);
            }
        }
    }

    if (tmax > tmin)
        fitToView_(tmin, tmax, fitX, fitY, marginInPixels);
}


void Timeline1DView::fitToView_(Double tmin, Double tmax, bool fitX, bool fitY, int marginInPixels)
{
    if (!tl_ || !(fitX || fitY)) return;

    Double
        deltax = std::max((Double)0.01, tmax - tmin),
        deltay,
        vmin, vmax;
    tl_->getMinMax(tmin, tmax, vmin, vmax);
    deltay = std::max((Double)0.01, vmax - vmin);

    if (fitX)
    {
        space_.setX( tmin );
        space_.setScaleX( deltax );
    }
    if (fitY)
    {
        space_.setY( vmin );
        space_.setScaleY( deltay );
    }

    Double mx = screen2time(marginInPixels) - screen2time(0),
           my = -(screen2value(marginInPixels) - screen2value(0));

    if (fitX)
    {
        space_.setX( space_.x() - mx );
        space_.setScaleX( space_.scaleX() + mx*2 );
    }
    if (fitY)
    {
        space_.setY( space_.y() - my );
        space_.setScaleY( space_.scaleY() + my*2 );
    }

    emit viewSpaceChanged(space_);
    update();
}

QRect Timeline1DView::handleRect_(const MATH::Timeline1d::Point& p, RectStyle_ rs)
{
    int r = handleRadius_;

    switch (rs)
    {
        case RS_HOVER: r = handleRadiusHovered_; break;
        case RS_SELECTED: r = handleRadiusSelected_; break;
        case RS_UPDATE: r = handleRadiusSelected_+1; break;
        case RS_NORMAL: break;
    }

    QRect rect(0,0,r*2,r*2);
    rect.moveTo(time2screen(p.t) - r,
                value2screen(p.val) - r);
    return rect;

}

QRect Timeline1DView::handleNumberRect_(
        const MATH::Timeline1d::Point& p)
{
    auto r = handleRect_(p, RS_NORMAL);
    r.moveTo(r.topLeft() + QPoint(12,-10));
    r.setSize(QSize(96,32));
    return r;
}

QRect Timeline1DView::derivativeHandleRect_(
        const MATH::Timeline1d::Point& p, int idx, RectStyle_ rs)
{
    int r = handleRadius_ * .8;

    switch (rs)
    {
        case RS_HOVER: r = handleRadiusHovered_; break;
        case RS_SELECTED: r = handleRadiusSelected_; break;
        case RS_UPDATE: r = handleRadiusSelected_+1; break;
        case RS_NORMAL: break;
    }

    Double xo = 0., yo = 0.,
           tw = screen2time(30) - screen2time(0),
           th = -tw;
    if (idx == 0)
    {
        xo -= tw;
        yo += p.d1 * th;
    }
    else
    {
        xo += tw;
        yo -= p.d1 * th;
    }

    // limit height of handle
    int Y = value2screen(p.val),
        curH = std::max(20, std::min(70, std::abs(value2screen(p.val+yo)-Y)));
    Double lim = screen2value(std::max(5,Y-curH)) - p.val;

    if (yo > lim)
        xo = xo / yo * lim, yo = lim;
    lim = screen2value(std::min(height()-6-r*2, Y+curH)) - p.val;
    if (yo < lim)
        xo = xo / yo * lim, yo = lim;

    QRect rect(0,0,r*2,r*2);
    rect.moveTo(time2screen(p.t + xo) - r,
                value2screen(p.val + yo) - r
                //std::max(5, std::min(height()-6-r*2, value2screen(y) - r ))
                );
    return rect;

}

void Timeline1DView::updateHandles_(const MATH::Timeline1d::Point& po)
{
    update(handleRect_(po, RS_UPDATE));
    if (po.hasDerivative())
    {
        auto r1 = derivativeHandleRect_(po, 0, RS_UPDATE),
             r2 = derivativeHandleRect_(po, 1, RS_UPDATE);
        update(r1.united(r2));
    }
    update(handleNumberRect_(po));
}

void Timeline1DView::updateAroundPoint_(const MATH::Timeline1d::Point &p)
{
    if (!tl_)
        return;

    auto first = tl_->find(p.t),
         last = first;

    if (first == tl_->getData().end())
        return;

    // determine number of neighbour points

    int expand_left = 2,
        expand_right = 2;

    if (p.type == MATH::TimelinePoint::SPLINE6)
    {
        expand_left = expand_right = 3;
    }

    // maximum window when in doubt
    int x1 = 0, x2 = width();

    // expand left
    int i=0;
    while ((i++)<expand_left && first != tl_->getData().begin())
        --first;

    if (i>expand_left && first != tl_->getData().end())
        x1 = time2screen(first->second.t) - handleRadiusSelected_ - 1;

    // expand right
    i=0;
    while ((i++)<expand_right)
    {
        auto next = last;
        ++next;
        if (next == tl_->getData().end())
            break;
        last = next;
    }
    if (i>expand_right && last != tl_->getData().end())
        x2 = time2screen(last->second.t) + handleRadiusSelected_ + 1;

    // wow, not clear if this is actually more efficient in any case
    // than a simple update()
    update(x1, 0, x2-x1, height());
}

void Timeline1DView::paintEvent(QPaintEvent * e)
{
    //qDebug() << space_.offsetX << "," << space_.offsetY << "   " << space_.scaleX << "," << space_.scaleY;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // -- background --

    p.setPen(Qt::NoPen);
    //p.setBrush(QBrush(QColor(palette().color(backgroundRole()))));
    p.setBrush(QBrush(QColor(50,50,50)));
    p.drawRect(e->rect());

    // -- grid --

    gridPainter_->setViewSpace(space_);
    gridPainter_->paint(p, e->rect());

    if (!tl_)
        return;

    // -- sequence curve --

    if (sequenceCurvePainter_)
    {
        sequenceCurvePainter_->setViewSpace(ospace_);
        sequenceCurvePainter_->paint(p, e->rect());
    }

    // -- curve --

    static_cast<TimelineCurveData*>(valuePainterData_)->timeline = tl_;
    valuePainter_->setViewSpace(viewSpace());
    valuePainter_->paint(p, e->rect());

    // -- handles --

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(255,255,0,200)));

    // get handles vertically on screen
    const int i0 = e->rect().left() - 1,
              i1 = e->rect().right() + 2;
    auto it0 = tl_->first(screen2time(i0-handleRadiusSelected_)),
         it1 = tl_->first(screen2time(i1+handleRadiusSelected_));

    Double ylim1 = screen2value(0),
           ylim0 = screen2value(height()-1);
    for (auto it = it0; it != it1 && it != tl_->getData().end(); ++it)
    {
        const MATH::Timeline1d::Point& po = it->second;

        if (po.val < ylim0 || po.val > ylim1)
            continue;

        const bool
                hovered = (hoverHash_ == it->first || moveHoverHash_ == it->first),
                derHovered = (derivativeHoverHash_ == it->first),
                selected = (selectHashSet_.contains(it->first));
        const int red = po.isDifferentiable() ? 0 : 35;
        const QColor selCol(200+red,255-red,150);

        if (selected)
            p.setBrush(QBrush(selCol));
        else
        if (hovered)
            p.setBrush(QBrush(QColor(220+red,220-red/2,150,200)));
        else
            p.setBrush(QBrush(QColor(200+red,200-red/2,100,200)));

        auto hr = handleRect_(po, hovered? RS_HOVER : RS_NORMAL);
        p.drawRect(hr);

        // derivative handle rects/positions
        auto dr1 = derivativeHandleRect_(
                        po, 0, derHovered && derivativeHoverIndex_ == 0
                                                     ? RS_HOVER : RS_NORMAL),
             dr2 = derivativeHandleRect_(
                        po, 1, derHovered && derivativeHoverIndex_ == 1
                                                     ? RS_HOVER : RS_NORMAL);
        if (selected)
        {
            // selection frame around handle
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(selCol));
            p.drawRect(handleRect_(po, RS_SELECTED));

            // derivative lines
            if (po.hasDerivative())
            {
                p.setPen(QPen(QColor(255,255,255,100)));
                p.drawLine(hr.center(), dr1.center());
                p.drawLine(hr.center(), dr2.center());
                p.setPen(Qt::NoPen);
            }

            // derivative handles
            if (po.isUserDerivative())
            {
                p.setBrush(QBrush(QColor(255,255,255,100)));
                p.drawRect(dr1);
                p.drawRect(dr2);
            }
        }

        // -- show numbers --

        if ((hovered && !derHovered))
        {
            auto r = handleNumberRect_(po);
            p.setPen(QPen(Qt::white));
            p.setBrush(Qt::NoBrush);
            //p.drawRect(r);
            p.drawText(r, 0, QString("t %1\nv %2").arg(po.t).arg(po.val));
            p.setPen(Qt::NoPen);
        }
    }

    // ---- sequence overpaint ------

    if (sequenceOverpaint_)
    {
        sequenceOverpaint_->setViewSpace(ospace_);
        sequenceOverpaint_->paint(p, e->rect());
    }

    // ------ selection frame -------

    if (action_ == A_SELECT_FRAME)
    {
        p.setPen(QPen(QColor(255,255,255)));
        p.setBrush(Qt::NoBrush);
        p.drawRect(selRect_);
    }
}

void Timeline1DView::clearHover_()
{
    auto wasHash = hoverHash_;
    hoverHash_ = moveHoverHash_ = MATH::Timeline1d::InvalidHash;

    // remove old flag
    if (wasHash != hoverHash_)
    {
        auto it1 = tl_->getData().lower_bound(wasHash);
        if (it1 != tl_->getData().end())
            updateHandles_(it1->second);
    }
}

void Timeline1DView::setHover_(const MATH::Timeline1d::Point & p, bool setDeriv)
{
    auto hash = MATH::Timeline1d::hash(p.t);

    if (!setDeriv)
    {
        if (hash == hoverHash_)
            return;
    }
    else
    {
        if (hash == derivativeHoverHash_)
            return;
    }

    if ((isHover_() || isDerivativeHover_()) && tl_)
    {
        // remove old flag
        auto it1 = tl_->getData().lower_bound(hoverHash_);
        if (it1 != tl_->getData().end())
            updateHandles_(it1->second);
        it1 = tl_->getData().lower_bound(derivativeHoverHash_);
        if (it1 != tl_->getData().end())
            updateHandles_(it1->second);
    }

    if (setDeriv)
        derivativeHoverHash_ = hash;
    else
        hoverHash_ = hash;

    updateHandles_(p);/*
    update(handleRect_(p, RS_UPDATE));
    update(derivativeHandleRect_(p, 0, RS_UPDATE));
    update(derivativeHandleRect_(p, 1, RS_UPDATE));*/
}

bool Timeline1DView::isHover_() const
{
    return hoverHash_ != MATH::Timeline1d::InvalidHash;
}

bool Timeline1DView::isDerivativeHover_() const
{
    return derivativeHoverHash_ != MATH::Timeline1d::InvalidHash
            && derivativeHoverIndex_ >= 0;
}

MATH::Timeline1d::TpList::iterator Timeline1DView::hoverPoint_()
{
    if (!tl_)
        return MATH::Timeline1d::TpList::iterator();

    return tl_->getData().lower_bound(hoverHash_);
}

void Timeline1DView::unselect()
{
    clearSelect_();
    clearHover_();
    action_ = A_NOTHING;
    update();
}

void Timeline1DView::clearSelect_()
{
    if (!isSelected())
        return;

    // remove old flags
    for (auto &i : selectHashSet_)
    {
        auto it1 = tl_->getData().lower_bound(i);
        if (it1 != tl_->getData().end())
            updateHandles_(it1->second);
    }
    selectHashSet_.clear();
}

void Timeline1DView::addSelect_(const MATH::Timeline1d::Point & p, bool do_swap)
{
    auto hash = MATH::Timeline1d::hash(p.t);

    if (selectHashSet_.contains(hash))
    {
        if (do_swap)
        {
            selectHashSet_.remove(hash);
            updateHandles_(p);
        }
        return;
    }

    selectHashSet_.insert(hash);

    updateHandles_(p);
}

void Timeline1DView::addSelect_(const QRect &rect, bool do_swap)
{
    if (!tl_)
        return;

    const Double xmin = screen2time(rect.left()),
                 xmax = screen2time(rect.right()),
                 ymin = screen2value(rect.bottom()),
                 ymax = screen2value(rect.top());

    auto it = tl_->first(xmin);

    for (; it != tl_->getData().end(); ++it)
    {
        if (it->second.t > xmax)
            break;

        if (it->second.val >= ymin && it->second.val <= ymax)
            addSelect_(it->second, do_swap);
    }
}


void Timeline1DView::wheelEvent(QWheelEvent * e)
{
    if (action_ == A_DRAG_SELECTED)
        return;

    changeScale_(e->x(), e->y(),
                 e->angleDelta().y() > 0? (1.0 - zoomChange_)
                                        : e->angleDelta().y() < 0? (1.0 + zoomChange_) : 1.0,
                 e->angleDelta().x() > 0? (1.0 - zoomChange_)
                                        : e->angleDelta().x() < 0? (1.0 + zoomChange_) : 1.0
                );

    e->accept();
    update();
}

QCursor Timeline1DView::defaultCursor_() const
{
    return Qt::ArrowCursor;
}

void Timeline1DView::keyPressEvent(QKeyEvent * e)
{
    // change to select-frame mode
    if (e->modifiers() & modifierSelectFrame_)
    {
        if (!isHover_())
        {
            if (action_ == A_START_SELECT_FRAME)
            {
                setCursor(defaultCursor_());
                action_ = A_NOTHING;
            }
            else
            {
                setCursor(Qt::CrossCursor);
                action_ = A_START_SELECT_FRAME;
            }
        }
    }

    // select all
    if (e->key() == Qt::Key_A && (e->modifiers() & Qt::CTRL))
    {
        selectAll_();
        return;
    }

    QWidget::keyPressEvent(e);
}

void Timeline1DView::keyReleaseEvent(QKeyEvent * e)
{
    // change to select-frame mode
    if (e->modifiers() & modifierSelectFrame_)
    {
        if (!isHover_())
        {
            if (action_ == A_START_SELECT_FRAME)
            {
                setCursor(defaultCursor_());
                action_ = A_NOTHING;
            }
        }
    }

    QWidget::keyReleaseEvent(e);
}

void Timeline1DView::mouseMoveEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    // mouse coords in timeline space
    const
    Double x = screen2time(e->x()),
           y = screen2value(e->y()),
    // handle radius in timeline space
           rx = screen2time(e->x()+handleRadius_) - x,
           ry = screen2value(e->y()-handleRadius_) - y;
    // one pixel in timeline space
           //onePixT = screen2time(1) - screen2time(0);

    // --- drag position / space ---

    if (action_ == A_DRAG_SPACE)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        if (!(options_ & O_MoveViewX))
            dx = 0;
        if (!(options_ & O_MoveViewY))
            dy = 0;

        if (dx || dy)
        {
            space_.setX( dragStartSpace_.x() - dx );
            space_.setY( dragStartSpace_.y() - dy );

            emit viewSpaceChanged(space_);
            update();
        }

        e->accept();
        return;
    }

    // --- drag selected points ---

    if (action_ == A_DRAG_SELECTED)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        // only move vertically on CTRL
        if (e->modifiers() & modifierMoveVert_)
            dx = 0;

        {
            {
                Locker_ lock(this);
                moveSelected_(dx, dy);
            }
            emit timelineChanged();
        }

        e->accept();
        return;
    }

    // --- change derivatives ---

    if (action_ == A_DRAG_DERIVATIVE)
    {
        // mouse delta in timeline space
        Double dx = screen2time(e->x()) - screen2time(dragStart_.x()),
               dy = screen2value(e->y()) - screen2value(dragStart_.y());

        // only move vertically on CTRL
        if (e->modifiers() & modifierMoveVert_)
            dx = 0;

        {
            {
                Locker_ lock(this);
                changeDerivativesSelected_(dx, dy
                                           / (screen2time(30)-screen2time(0)));
            }
            emit timelineChanged();
        }

        e->accept();
        return;
    }

    // --- select frame ---

    if (action_ == A_SELECT_FRAME)
    {
        update(selRect_.adjusted(-1,-1,1,1));
        // adjust rectangle
        selRect_.setLeft(std::min(e->x(), dragStart_.x()));
        selRect_.setTop(std::min(e->y(), dragStart_.y()));
        selRect_.setRight(std::max(e->x(), dragStart_.x()));
        selRect_.setBottom(std::max(e->y(), dragStart_.y()));
        update(selRect_.adjusted(-1,-1,1,1));

        e->accept();
        return;
    }

    // -- no action defined here --


    // --------- on hover over --------------

    auto oldDerivativeHoverHash = derivativeHoverHash_;

    derivativeHoverIndex_ = -1;
    hoverCurveHash_ = MATH::Timeline1d::InvalidHash;

    if (options_ & O_EditAll)
    {
        // hover over derivative rects?
        if (isSelected() && (options_ & O_ChangeDerivative))
        {
            Double ylim1 = screen2value(0),
                   ylim0 = screen2value(height()-1);
            for (const auto& h : selectHashSet_)
            {
                auto it1 = tl_->getData().lower_bound(h);
                if (it1 != tl_->getData().end())
                {
                    const MATH::Timeline1d::Point& po = it1->second;
                    if (po.val < ylim0 || po.val > ylim1)
                        continue;

                    auto r1 = derivativeHandleRect_(po, 0, RS_NORMAL),
                         r2 = derivativeHandleRect_(po, 1, RS_NORMAL);
                    if (r1.contains(e->pos()))
                        derivativeHoverIndex_ = 0;
                    else if (r2.contains(e->pos()))
                        derivativeHoverIndex_ = 1;
                    if (derivativeHoverIndex_ >= 0)
                    {
                        setHover_(po, true);
                        setCursor(Qt::OpenHandCursor);
                        setStatusTip(
                            tr("Left-click & drag to change derivative"));
                        e->accept();
                        return;
                    }
                }
            }

            // update old derivative hover state
            derivativeHoverHash_ = MATH::Timeline1d::InvalidHash;
            if (oldDerivativeHoverHash != MATH::Timeline1d::InvalidHash)
            {
                auto it1 = tl_->getData().lower_bound(derivativeHoverHash_);
                if (it1 != tl_->getData().end())
                    updateHandles_(it1->second);
            }
            setCursor(Qt::ArrowCursor);
        }


        // hover over points?

        auto oldHoverHash = hoverHash_;

        // get the timeline points in range
        auto it = tl_->first(x-rx),
             last = tl_->next_after(x+rx);
        // iterate over all of them
        for (; it != tl_->getData().end() && it != last; ++it)
        {
            // hover a handle?
            if (x >= it->second.t - rx
             && x <= it->second.t + rx
             && y >= it->second.val - ry
             && y <= it->second.val + ry)
            {
                if (it->first == oldHoverHash)
                    return;

                // set new hover point
                setHover_(it->second, false);

                setCursor(Qt::OpenHandCursor);

                QString tip;
                if (options_ & O_EditAll)
                    tip = tr("Left-click to select (hold %1 for multi-select) / ")
                            .arg(enumName(modifierMultiSelect_));
                if (options_ & O_MovePoints)
                    tip += tr("Left-click & drag to move point(s) / ");
                if (options_ & O_EditAll)
                    tip += tr("Right-click for context menu");
                setStatusTip_(tip);

                e->accept();
                return;
            }
        }
    }

    // unhover
    clearHover_();

    // ----- hover on curve -----

    if (options_ & O_AddRemovePoints)
    {
        auto it = tl_->closest(x);
        if (it != tl_->getData().end())
        {
            // get the point left of mouse
            if (it->second.t > x && it != tl_->getData().begin())
                --it;

            if (it->second.t <= x)
            {
                // get value and derivative at mousepos
                const Double vx = tl_->get(x),
                            vx0 = tl_->get(x - 0.01),
                            vx1 = tl_->get(x + 0.01),
                            dx = (vx1 - vx0) / 0.01;

                // get mouse distance to curve
                const int dist_scr = abs(value2screen(vx) - e->y());
                // if within range (+derivative)
                if (dist_scr <= 2+abs(dx))
                {
                    setCursor(Qt::CrossCursor);
                    hoverCurveHash_ = it->first;
                    setStatusTip_(tr("Left-click to add a new point"));
                    e->accept();
                    return;
                }
            }
        }
    }

    // ---- hover over empty space -----

    QString tip;
    if (options_ & O_MoveView)
        tip = tr("Left-click & drag to change position / ");
    if (options_ & O_EditAll)
        tip += tr("%1 + Left-click for selection frame / ")
                .arg(enumName(modifierSelectFrame_));
    if (options_ & O_AddRemovePoints)
        tip += tr("Double-click to add point / ");
    setStatusTip_(tip + tr("Right-click for context menu"));

    // --- start selection frame ---
    if ((e->modifiers() & modifierSelectFrame_)
        && (options_ & O_EditAll))
    {
        action_ = A_START_SELECT_FRAME;
        setCursor(Qt::CrossCursor);
    }
    else

    // -- start dragging space --
    if (options_ & O_MoveView)
    {
        action_ = A_START_DRAG_SPACE;
        if (!(options_ & O_MoveViewX))
            setCursor(Qt::SizeVerCursor);
        else if (!(options_ & O_MoveViewY))
            setCursor(Qt::SizeHorCursor);
        else
            setCursor(Qt::SizeAllCursor);
    }
    else
        setCursor(Qt::ArrowCursor);
}

void Timeline1DView::mousePressEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    const bool isMultiSel = e->modifiers() & modifierMultiSelect_;

    // ---- click on curve ----

    if (hoverCurveHash_ != MATH::Timeline1d::InvalidHash)
    {
        if (e->button() == Qt::LeftButton)
        {
            {
                Locker_ lock(this);
                addPoint_(screen2time(e->x()), screen2value(e->y()));
            }
            emit timelineChanged();
            e->accept();
            // do not return but go on to click-on-point (start dragging)
        }
        else
            return;

    }

    // ---- click on derivative handle ---

    if (isDerivativeHover_())
    {
        if (e->button() == Qt::LeftButton
            && (options_ & O_ChangeDerivative))
        {
            /*
            auto it1 = tl_->getData().lower_bound(derivativeHoverHash_);
            if (it1 != tl_->getData().end())
            {
                const MATH::Timeline1d::Point& po = it1->second;
            }*/
            action_ = A_DRAG_DERIVATIVE;
            dragStart_ = e->pos();
            derivativeOriginalList_.clear();
            for (auto &h : selectHashSet_)
            {
                auto it = tl_->getData().lower_bound(h);
                if (it != tl_->getData().end())
                    derivativeOriginalList_.insert(
                                std::make_pair(it->first, it->second));
            }
            e->accept();
            return;
        }
    }

    // ---- click on point ----

    if (isHover_())
    {
        if (e->button() == Qt::LeftButton)
        {
            auto point = hoverPoint_();
            // fail-safe (should be a point though)
            if (point == tl_->getData().end())
                return;

            // keep selection when shift pressed
            if (!isMultiSel
                // or when already selected
             && !selectHashSet_.contains(hoverHash_))
                clearSelect_();

            addSelect_(point->second, isMultiSel);

            if (options_ & O_MovePoints)
            {
                action_ = A_DRAG_SELECTED;
                dragStart_ = e->pos();
                setCursor(Qt::ClosedHandCursor);
                setStatusTip_(tr("Drag to move, hold %1 to move only vertically")
                              .arg(enumName(modifierMoveVert_)));

                // copy the points to drag set
                dragPoints_.clear();
                for (auto &h : selectHashSet_)
                {
                    auto it = tl_->getData().lower_bound(h);
                    if (it != tl_->getData().end())
                        dragPoints_.push_back(DragPoint_(it));
                }
            }

            e->accept();
            return;
        }

        if (e->button() == Qt::RightButton)
        {
            // clear selection when clicking on unselected
            if (!(e->modifiers() & modifierMultiSelect_)
                && !selectHashSet_.contains(hoverHash_))
                clearSelect_();

            e->accept();
            slotPointContextMenu_();
            return;
        }
    }

    // ----- click in empty space -----

    // drag space / position
    if (e->button() == Qt::LeftButton)
    {
        if (action_ == A_START_SELECT_FRAME)
        {
            action_ = A_SELECT_FRAME;
            dragStart_ = e->pos();
            selRect_ = QRect(e->pos(), QSize(1,1));
            update(selRect_.adjusted(-1,-1,1,1));
            e->accept();
            return;
        }

        if (action_ == A_START_DRAG_SPACE)
        {
            action_ = A_DRAG_SPACE;
            dragStart_ = e->pos();
            dragStartSpace_ = space_;
            e->accept();
            return;
        }
    }

    if (e->button() == Qt::RightButton)
    {
        slotEmptyContextMenu_();
        e->accept();
        return;
    }
}

void Timeline1DView::mouseDoubleClickEvent(QMouseEvent * e)
{
    // --- double-click on point ---
    if (isHover_())
    {
        e->accept();
        return;
    }

    // --- double-click on derivative handle ---
    if (isDerivativeHover_())
    {
        e->accept();
        return;
    }

    // --- double-click in empty space ---

    if (e->button() == Qt::LeftButton)
    {
        if (options_ & O_AddRemovePoints)
        {
            {
                Locker_ lock(this);
                addPoint_(screen2time(e->x()), screen2value(e->y()));
            }
            emit timelineChanged();
            // go to drag action imidiately
            mousePressEvent(e);
            return;
        }
    }
}

void Timeline1DView::mouseReleaseEvent(QMouseEvent * e)
{
    const bool isMultiSel = e->modifiers() & modifierMultiSelect_;

    if (action_ == A_SELECT_FRAME)
    {
        if (!isMultiSel)
            clearSelect_();

        addSelect_(selRect_, isMultiSel);

        update(selRect_.adjusted(-1,-1,1,1));
    }

    // back to hover over point
    if (action_ == A_DRAG_SELECTED)
        setCursor(Qt::OpenHandCursor);

    action_ = A_NOTHING;
    moveHoverHash_ = MATH::Timeline1d::InvalidHash;

    // update status tips
    mouseMoveEvent(e);
}


void Timeline1DView::selectAll_()
{
    selectHashSet_.clear();

    if (!tl_ || !(options_ & O_EditAll))
        return;

    for (auto &it : tl_->getData())
        selectHashSet_.insert(it.first);

    update();
}

void Timeline1DView::selectDirection_(int dir)
{
    if (!(options_ & O_EditAll))
        return;

    const Double x = screen2time(popupClick_.x()),
                 y = screen2value(popupClick_.y());

    switch (dir)
    {
    case Qt::LeftArrow:
        for (auto i = tl_->getData().begin(); i!=tl_->getData().end(); ++i)
        {
            if (i->second.t <= x)
                selectHashSet_.insert(i->first);
            if (i->second.t > x)
                break;
        }
    break;
    case Qt::RightArrow:
        for (auto i = tl_->first(x); ; ++i)
            if (i == tl_->getData().end())
                break;
            else
                selectHashSet_.insert(i->first);
    break;
    case Qt::UpArrow:
        for (auto i = tl_->getData().begin(); i!=tl_->getData().end(); ++i)
        {
            if (i->second.val >= y)
                selectHashSet_.insert(i->first);
        }
    break;
    case Qt::DownArrow:
        for (auto i = tl_->getData().begin(); i!=tl_->getData().end(); ++i)
        {
            if (i->second.val <= y)
                selectHashSet_.insert(i->first);
        }
    break;
    default: return;
    }

    update();
}

Double Timeline1DView::limitX_(Double time) const
{
    // limit to screen
    if (!(options_ & O_MoveViewX) && !(options_ & O_ZoomViewX))
        return std::max(screen2time(0),
                        std::min(screen2time(width()-1),
                            time ));
    else
        return time;
}

Double Timeline1DView::limitY_(Double value) const
{
    // limit to screen
    if (!(options_ & O_MoveViewY) && !(options_ & O_ZoomViewY))
        return std::max(screen2value(height()-1),
                        std::min(screen2value(0),
                            value ));
    else
        return value;
}

void Timeline1DView::moveSelected_(Double dx, Double dy)
{
    if (!tl_)
        return;

    if (!(options_ & O_MovePointsX))
        dx = 0;
    if (!(options_ & O_MovePointsY))
        dy = 0;

    if (!(dx || dy))
        return;

    // only move vertically?
    if (std::abs(dx) < MATH::Timeline1d::timeQuantum())
    {
        for (auto &p : dragPoints_)
        {
            if (p.valid)
            {
                p.it->second.val = limitY_( p.oldp.val + dy );
            }
            updateAroundPoint_(p.it->second);
        }

        // recalc derivatives
        for (auto &p : dragPoints_)
        {
            if (p.valid)
                updateDerivatives_(p.it);
        }

        return;
    }

    // potentially we change all the points so
    // we need to rebuild the selection hash
    selectHashSet_.clear();


    for (auto &p : dragPoints_)
    {
        if (!p.valid) continue;

        // create a new point
        Double newTime;
        auto newp = tl_->add(newTime = limitX_(p.oldp.t + dx),
                             limitY_(p.oldp.val + dy), p.oldp.type);

        // was there a point already?
        if (newp == 0)
        {
            auto it2 = tl_->find(limitX_(p.oldp.t + dx));

            // same point? then change value only
            if (it2 == p.it)
            {
                p.it->second.val = limitY_( p.oldp.val + dy );
                //p.it->second.d1 = p.oldp.d1;
                updateAroundPoint_(p.it->second);
            }

            // update selection hash
            selectHashSet_.insert(p.it->first);
            continue;
        }
        else
        {
            // also copy these values
            // TODO because they are not in the Timeline1d interface
            newp->d1 = p.oldp.d1;

            if (hoverHash_ == p.oldp.hash())
            {
                moveHoverHash_ = newp->hash();
            }
        }

        // delete previous point
        const Double t0 = p.it->second.t;
        updateAroundPoint_(p.it->second);
        tl_->getData().erase(p.it);
        p.valid = false;
        // update derivative around erased point
        auto it3 = tl_->next_after(t0);
        if (it3 != tl_->getData().end())
            updateDerivatives_(it3, 2);

        // keep the new point/iterator to find it next time
        p.newp = *newp;
        p.it = tl_->find(p.newp.t);

        if (p.it != tl_->getData().end())
        {
            p.valid = true;

            // update selection hash
            selectHashSet_.insert(p.it->first);
            updateAroundPoint_(p.it->second);
        }
    }

    // recalc derivatives
    for (auto &p : dragPoints_)
    {
        if (p.valid)
            updateDerivatives_(p.it);
    }
}

void Timeline1DView::changeDerivativesSelected_(Double /*dx*/, Double dy)
{
    if (!tl_)
        return;

    if (!(options_ & O_ChangeDerivative))
        return;

    if (derivativeHoverIndex_ == 0)
        dy = -dy;

    for (const auto& it : selectHashSet_)
    {
        auto it1 = tl_->getData().lower_bound(it);
        if (it1 == tl_->getData().end())
            continue;
        MATH::Timeline1d::Point& po = it1->second;

        it1 = derivativeOriginalList_.lower_bound(it);
        if (it1 == derivativeOriginalList_.end())
            continue;
        const MATH::Timeline1d::Point& oldPo = it1->second;

        po.d1 = oldPo.d1 + dy;
        //qDebug() << po.d1;

        updateAroundPoint_(po);
    }
}


void Timeline1DView::slotPointContextMenu_()
{
    if (!isHover_() || !tl_)
        return;

    // get the hovered point
    auto pointIt = tl_->getData().lower_bound(hoverHash_);
    if (pointIt == tl_->getData().end())
        return;

    QAction * a;

    // main popup
    QMenu * pop = new QMenu(this);
    connect(pop, SIGNAL(triggered(QAction*)), pop, SLOT(deleteLater()));

    // point type submenu
    QMenu * pointpop = new QMenu(pop);
    for (int i=1; i<MATH::TimelinePoint::MAX; ++i)
    {
        auto type = (MATH::TimelinePoint::Type)i;

        a = new QAction(MATH::TimelinePoint::getName(type), pointpop);
        pointpop->addAction(a);
        a->setCheckable(true);
        a->setChecked(pointIt->second.type == type);
        connect(a, &QAction::triggered, [=]()
        {
            {
                Locker_ lock(this);
                changePointType_(type);
            }
            emit timelineChanged();
            update();
        });
    }

    // --- single point popup ---

    if (selectHashSet_.size() < 2)
    {
        if (options_ & O_ChangePointType)
        {
            pop->addMenu(pointpop);
            pointpop->setTitle(tr("change point type"));
            pointpop->setStatusTip(tr("Changes the interpolation type between the selected "
                                      "and the next point"));
        }

        if (options_ & O_AddRemovePoints)
        {
            a = new QAction(tr("delete point"), pop);
            a->setStatusTip(tr("Deletes the selected point"));
            pop->addAction(a);
            connect(a, &QAction::triggered, [=]()
            {
                const Double t = pointIt->second.t;
                {
                    Locker_ lock(this);
                    tl_->remove(t);
                }
                emit timelineChanged();
                selectHashSet_.remove(MATH::Timeline1d::hash(t));
                update();
            });
        }
    }

    // --- selected points popup ---
    else
    {
        if (options_ & O_ChangePointType)
        {
            pop->addMenu(pointpop);
            pointpop->setTitle(tr("change points type"));
            pointpop->setStatusTip(tr("Changes the interpolation type for all selected points"));
        }

        if (options_ & O_AddRemovePoints)
        {
            a = new QAction(tr("delete points"), pop);
            a->setStatusTip(tr("Deletes all selected points"));
            pop->addAction(a);
            connect(a, &QAction::triggered, [=]()
            {
                {
                    Locker_ lock(this);
                    for (auto h : selectHashSet_)
                        tl_->remove(h);

                    tl_->setAutoDerivative();
                }
                emit timelineChanged();

                selectHashSet_.clear();
                update();
            });
        }
    }

    if (isSelected())
    {
        if (!pop->isEmpty())
            pop->addSeparator();

        a = new QAction(tr("copy selection"), pop);
        a->setStatusTip(tr("Copies the selected points to the clipboard"));
        pop->addAction(a);
        connect(a, SIGNAL(triggered()), this, SLOT(copySelection()));
    }

    if (pop->isEmpty())
    {
        pop->deleteLater();
        return;
    }

    popupClick_ = mapFromGlobal(QCursor::pos());

    pop->exec(QCursor::pos());
}

void Timeline1DView::slotEmptyContextMenu_()
{
    // XXX Mhh, action's statusTips are not displayed in mainwindow!

    if (!tl_)
        return;

    QAction * a;

    // main popup
    QMenu * pop = new QMenu(this);
    connect(pop, SIGNAL(triggered(QAction*)), pop, SLOT(deleteLater()));

    if (isSelected())
    {

        a = new QAction(tr("unselect"), pop);
        a->setStatusTip(tr("Clears the current selection"));
        pop->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            clearSelect_();
        });
    }

    if (options_ & O_EditAll)
    {
        a = new QAction(tr("select all"), pop);
        a->setStatusTip(tr("Selects all points in the timeline"));
        pop->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            selectAll_();
        });

        // --- selection subpop ---
        QMenu * selectpop = new QMenu(pop);
        pop->addMenu(selectpop);
        selectpop->setTitle(tr("add to selection"));
        selectpop->setStatusTip(tr("Selects points according to their positions and values"));

        a = new QAction(tr("all points left"), selectpop);
        a->setStatusTip(tr("Selects all points left of the chosen position"));
        selectpop->addAction(a);
        connect(a, &QAction::triggered, [=]() { selectDirection_(Qt::LeftArrow); });
        a = new QAction(tr("all points right"), selectpop);
        a->setStatusTip(tr("Selects all points right of the chosen position"));
        selectpop->addAction(a);
        connect(a, &QAction::triggered, [=]() { selectDirection_(Qt::RightArrow); });
        a = new QAction(tr("all points above"), selectpop);
        a->setStatusTip(tr("Selects all points above the chosen position"));
        selectpop->addAction(a);
        connect(a, &QAction::triggered, [=]() { selectDirection_(Qt::UpArrow); });
        a = new QAction(tr("all points below"), selectpop);
        a->setStatusTip(tr("Selects all points below the chosen position"));
        selectpop->addAction(a);
        connect(a, &QAction::triggered, [=]() { selectDirection_(Qt::DownArrow); });


        pop->addSeparator();

        if (!tl_->empty())
        {
            a = new QAction(tr("copy timeline"), pop);
            a->setStatusTip(tr("Copies the whole timeline to the clipboard"));
            pop->addAction(a);
            connect(a, SIGNAL(triggered()), this, SLOT(copyAll()));
        }

        if (isSelected())
        {
            a = new QAction(tr("copy selection"), pop);
            a->setStatusTip(tr("Copies the selected points to the clipboard"));
            pop->addAction(a);
            connect(a, SIGNAL(triggered()), this, SLOT(copySelection()));
        }

        const ClipboardType ctype = isTimelineInClipboard();
        if (ctype != C_NONE)
        {
            a = new QAction(ctype == C_SELECTION? tr("paste selection here") : tr("paste timeline"), pop);
            a->setStatusTip(ctype == C_SELECTION?
                                tr("Pastes the points from clipboard points at the chosen position")
                              : tr("Replaces the current timeline with the timeline in the clipboard"));
            pop->addAction(a);
            connect(a, SIGNAL(triggered()), this, SLOT(paste()));
        }
    }


    // ----------
    pop->addSeparator();

    if (options_ & O_ChangeViewAll)
    {
        if (options_ & O_ZoomViewX)
        {
            a = new QAction(tr("fit time to view"), pop);
            a->setStatusTip(tr("Changes the x-scale of the view so that all points are visible"));
            pop->addAction(a);
            connect(a, &QAction::triggered, [=]() { fitToView(true, false); } );
        }

        if (options_ & O_ZoomViewY)
        {
            a = new QAction(tr("fit values to view"), pop);
            a->setStatusTip(tr("Changes the y-scale of the view so that all points are visible"));
            pop->addAction(a);
            connect(a, &QAction::triggered, [=]() { fitToView(false, true); } );
        }

        if ((options_ & O_ZoomView) == O_ZoomView)
        {
            a = new QAction(tr("fit to view"), pop);
            a->setStatusTip(tr("Changes the scale of the view so that all points are visible"));
            pop->addAction(a);
            connect(a, &QAction::triggered, [=]() { fitToView(true, true); } );
        }

        if (isSelected())
        {
            if (options_ & O_ZoomViewX)
            {
                a = new QAction(tr("fit selection time to view"), pop);
                a->setStatusTip(tr("Changes the x-scale of the view so that all selected points are visible"));
                pop->addAction(a);
                connect(a, &QAction::triggered, [=]() { fitSelectionToView(true, false); } );
            }

            if (options_ & O_ZoomViewY)
            {
                a = new QAction(tr("fit selection value to view"), pop);
                a->setStatusTip(tr("Changes the y-scale of the view so that all selected points are visible"));
                pop->addAction(a);
                connect(a, &QAction::triggered, [=]() { fitSelectionToView(false, true); } );
            }

            if (options_ & O_ZoomView)
            {
                a = new QAction(tr("fit selection to view"), pop);
                a->setStatusTip(tr("Changes the scale of the view so that all selected points are visible"));
                pop->addAction(a);
                connect(a, &QAction::triggered, [=]() { fitSelectionToView(true, true); } );
            }
        }
    }

    if (pop->isEmpty())
    {
        pop->deleteLater();
        return;
    }

    popupClick_ = mapFromGlobal(QCursor::pos());

    pop->popup(QCursor::pos());
}

void Timeline1DView::changePointType_(MATH::TimelinePoint::Type t)
{
    if (!tl_ || !(options_ & O_ChangePointType))
        return;

    if (isSelected())
    {
        for (auto &h : selectHashSet_)
        {
            auto pointIt = tl_->getData().lower_bound(h);
            if (pointIt != tl_->getData().end())
            {
                pointIt->second.type = t;
                updateDerivatives_(pointIt);
            }
        }
    }
    else if (isHover_())
    {
        auto pointIt = tl_->getData().lower_bound(hoverHash_);
        if (pointIt != tl_->getData().end())
        {
            pointIt->second.type = t;
            updateDerivatives_(pointIt);
        }
    }
}

void Timeline1DView::addPoint_(Double t, Double v)
{
    if (!tl_ || !(options_ & O_AddRemovePoints))
        return;

    auto p = tl_->add(t, v);

    if (!p)
        return;

    clearSelect_();
    addSelect_(*p);
    setHover_(*p, false);
    hoverCurveHash_ = MATH::Timeline1d::InvalidHash;

    setCursor(Qt::OpenHandCursor);

    updateDerivatives_(tl_->find(t));
    updateAroundPoint_(*p);
}

void Timeline1DView::updateDerivatives_(MATH::Timeline1d::TpList::iterator p, int lr)
{
    if (!tl_)
        return;

    auto it = p;
    int i = 0;

    while (it != tl_->getData().end() && (i++) <= lr)
    {
        if (it->second.isAutoDerivative())
            tl_->setAutoDerivative(it);

        if (it == tl_->getData().begin())
            break;
        --it;
    }

    it = p;
    i = 0;

    while (it != tl_->getData().end() && (i++) <= lr)
    {
        if (it->second.isAutoDerivative())
            tl_->setAutoDerivative(it);
        ++it;
    }
}



// ----------------------- CLIPBOARD ------------------------

Timeline1DView::ClipboardType Timeline1DView::isTimelineInClipboard()
{
    const auto list = QApplication::clipboard()->mimeData()->formats();
    if (list.contains("mo/timeline"))
        return C_WHOLE;
    if (list.contains("mo/timeline-sel"))
        return C_SELECTION;
    return C_NONE;
}

void Timeline1DView::copyAll()
{
    if (!tl_)
        return;

    QByteArray bytes;
    IO::DataStream stream(&bytes, QIODevice::WriteOnly);

    tl_->serialize(stream);

    auto data = new QMimeData();
    data->setData("mo/timeline", bytes);

    QApplication::clipboard()->setMimeData(data);
}

void Timeline1DView::copySelection()
{
    if (!tl_ || !isSelected())
        return;

    QByteArray bytes;
    IO::DataStream stream(&bytes, QIODevice::WriteOnly);

    // make a copy
    MATH::Timeline1d tl(*tl_);

    // and remove all unselected points
    std::set<MATH::Timeline1d::TpHash> times;
    for (auto &i : tl.getData())
        if (!selectHashSet_.contains(i.first))
            times.insert(i.first);
    for (auto i : times)
        tl.remove(i);

    tl.serialize(stream);

    auto data = new QMimeData();
    data->setData("mo/timeline-sel", bytes);

    QApplication::clipboard()->setMimeData(data);
}

void Timeline1DView::paste()
{
    if (!tl_)
        return;

    ClipboardType ctype = isTimelineInClipboard();

    if (ctype != C_NONE)
    {
        // read data
        QByteArray bytes =
            QApplication::clipboard()->mimeData()->data(
                    ctype == C_WHOLE? "mo/timeline" : "mo/timeline-sel");

        IO::DataStream stream(&bytes, QIODevice::ReadOnly);

        // create a temp timeline with the data
        MATH::Timeline1d tl;
        try
        {
            tl.deserialize(stream);
        }
        catch (const Exception& e)
        {
            QMessageBox::warning(this, "!", tr("Error pasting timeline data\n%1").arg(e.what()));
            return;
        }

        // paste selection
        if (ctype == C_SELECTION)
        {
            if (tl.getData().empty())
                return;

            const Double
                    clipTime = tl.getData().begin()->second.t,
                    clipVal = tl.getData().begin()->second.val,
                    insertTime = screen2time(popupClick_.x()) - clipTime,
                    insertVal = screen2value(popupClick_.y()) - clipVal;

            clearSelect_();
            {
                Locker_ lock(this);
                for (auto &i : tl.getData())
                {
                    auto p =
                        tl_->add(limitX_(i.second.t + insertTime),
                                 limitY_(i.second.val + insertVal),
                                 i.second.type);
                    if (p)
                        addSelect_(*p);
                }
            }
            emit timelineChanged();
        }
        // whole timeline
        else
        {
            {
                Locker_ lock(this);
                *tl_ = tl;
            }
            emit timelineChanged();
        }

        update();
    }
}

} // namespace GUI
} // namespace MO
