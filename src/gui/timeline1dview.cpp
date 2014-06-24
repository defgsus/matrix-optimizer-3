/** @file

    @brief widget for Timeline1D

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/24</p>
*/

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>

#include "timeline1dview.h"

namespace MO {
namespace GUI {

Timeline1DView::Timeline1DView(Timeline1D * tl, QWidget *parent)
    :   QWidget     (parent),
        tl_         (tl),

        overPaint_  (4),
        handleRadius_(3),
        handleRadiusHovered_(4),
        handleRadiusSelected_(6),
        zoomChange_ (0.05),

        hoverHash_  (Timeline1D::InvalidHash),
        hoverCurveHash_(Timeline1D::InvalidHash),
        action_     (A_NOTHING)
{
    space_.scaleX = 10;

    setMinimumSize(100,60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Timeline1DView::setTimeline(Timeline1D *timeline)
{
    tl_ = timeline;

    update();
}

Double Timeline1DView::screen2time(Double x) const
{
    return x / width() * space_.scaleX + space_.offsetX;
}

Double Timeline1DView::screen2value(Double y) const
{
    return (height() - 1 - y) / height() * space_.scaleY + space_.offsetY;
}

int Timeline1DView::time2screen(Double time) const
{
    return (time - space_.offsetX) / space_.scaleX * width();
}

int Timeline1DView::value2screen(Double val) const
{
    return height() - 1 - (val - space_.offsetY) * height() / space_.scaleY;
}

void Timeline1DView::changeScale_(int scrx, int scry, Double fx, Double fy)
{
    const Double
        tx = (Double)scrx / width(),
        ty = 1.0 - (Double)scry / height();

    ViewSpace olds(space_);

    space_.scaleX *= fx;
    space_.scaleY *= fy;

    const Double
        changex = (olds.scaleX - space_.scaleX),
        changey = (olds.scaleY - space_.scaleY);

    space_.offsetX += changex * tx;
    space_.offsetY += changey * ty;
}

void Timeline1DView::fitToView(int marginInPixels)
{
    if (!tl_) return;
    fitToView_(tl_->tmin(), tl_->tmax(), marginInPixels);
}

void Timeline1DView::fitSelectionToView(int marginInPixels)
{
    if (!tl_ || !isSelected_()) return;

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
        fitToView_(tmin, tmax, marginInPixels);
}


void Timeline1DView::fitToView_(Double tmin, Double tmax, int marginInPixels)
{
    if (!tl_) return;

    Double
        deltax = std::max((Double)0.01, tmax - tmin),
        deltay,
        vmin, vmax;
    tl_->getMinMax(tmin, tmax, vmin, vmax);
    deltay = std::max((Double)0.01, vmax - vmin);

    space_.offsetX = tmin;
    space_.offsetY = vmin;
    space_.scaleX = deltax;
    space_.scaleY = deltay;

    Double mx = screen2time(marginInPixels) - screen2time(0),
           my = -(screen2value(marginInPixels) - screen2value(0));

    space_.offsetX -= mx;
    space_.offsetY -= my;
    space_.scaleX += mx*2;
    space_.scaleY += my*2;

    update();
}

QRect Timeline1DView::handleRect_(const Timeline1D::Point& p, RectStyle_ rs)
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

    if (!tl_)
        return;

    // -- curve --

    overPaint_ = std::max(1, overPaint_);

    const int i0 = e->rect().left(),
              i1 = e->rect().right() + 1,
              im = (i1 - i0) * overPaint_;

    p.setPen(QPen(QColor(0,255,0)));
    p.setBrush(Qt::NoBrush);

    int y0=0, x0=0, y;
    for (int i=0; i<=im; ++i)
    {
        Double x = i0 + (Double)i / overPaint_;
        y = value2screen( tl_->get(screen2time(x)) );
        if (i!=0)
            p.drawLine(x0, y0, x, y);
        x0 = x;
        y0 = y;
    }

    // -- handles --

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(255,255,0,200)));

    auto it0 = tl_->first(screen2time(i0-handleRadiusSelected_)),
         it1 = tl_->first(screen2time(i1+handleRadiusSelected_));

    for (auto it = it0; it != it1 && it != tl_->getData().end(); ++it)
    {
        const bool hovered = (hoverHash_ == it->first);
        const bool selected = (selectHashSet_.contains(it->first));

        if (selected)
            p.setBrush(QBrush(QColor(200,255,200)));
        else
        if (hovered)
            p.setBrush(QBrush(QColor(250,250,150,200)));
        else
            p.setBrush(QBrush(QColor(200,200,100,200)));

        p.drawRect(handleRect_(it->second, hovered? RS_HOVER : RS_NORMAL));

        if (selected)
        {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(200,255,200)));
            p.drawRect(handleRect_(it->second, RS_SELECTED));
            p.setPen(Qt::NoPen);
        }
    }
}

void Timeline1DView::clearHover_()
{
    auto wasHash = hoverHash_;
    hoverHash_ = Timeline1D::InvalidHash;

    // remove old flag
    if (wasHash != hoverHash_)
    {
        auto it1 = tl_->getData().lower_bound(wasHash);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, RS_UPDATE));
    }
}

void Timeline1DView::setHover_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    if (hash == hoverHash_)
        return;

    if (isHover_() && tl_)
    {
        // remove old flag
        auto it1 = tl_->getData().lower_bound(hoverHash_);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, RS_UPDATE));
    }

    hoverHash_ = hash;
    update(handleRect_(p, RS_UPDATE));
}

bool Timeline1DView::isHover_() const
{
    return hoverHash_ != Timeline1D::InvalidHash;
}

Timeline1D::TpList::iterator Timeline1DView::hoverPoint_()
{
    if (!tl_)
        return Timeline1D::TpList::iterator();

    return tl_->getData().lower_bound(hoverHash_);
}

void Timeline1DView::clearSelect_()
{
    if (!isSelected_())
        return;

    // remove old flags
    for (auto &i : selectHashSet_)
    {
        auto it1 = tl_->getData().lower_bound(i);
        if (it1 != tl_->getData().end())
            update(handleRect_(it1->second, RS_UPDATE));
    }
    selectHashSet_.clear();
}

void Timeline1DView::addSelect_(const Timeline1D::Point & p)
{
    auto hash = Timeline1D::hash(p.t);

    if (selectHashSet_.contains(hash))
        return;

    selectHashSet_.insert(hash);

    update(handleRect_(p, RS_UPDATE));
}


void Timeline1DView::wheelEvent(QWheelEvent * e)
{
    changeScale_(e->x(), e->y(),
                 e->angleDelta().y() > 0? (1.0 - zoomChange_)
                                        : e->angleDelta().y() < 0? (1.0 + zoomChange_) : 1.0,
                 e->angleDelta().x() < 0? (1.0 - zoomChange_)
                                        : e->angleDelta().x() > 0? (1.0 + zoomChange_) : 1.0
                );

    e->accept();
    update();
}

void Timeline1DView::keyPressEvent(QKeyEvent * e)
{
    // select all
    if (e->key() == Qt::Key_A && (e->modifiers() & Qt::CTRL))
    {
        selectAll_();
        return;
    }

    QWidget::keyPressEvent(e);
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

        space_.offsetX = dragStartSpace_.offsetX - dx;
        space_.offsetY = dragStartSpace_.offsetY - dy;

        update();

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
        if (e->modifiers() & Qt::CTRL)
            dx = 0;

        moveSelected_(dx, dy);

        e->accept();
        return;
    }

    // -- no action defined yet --
    //Q_ASSERT(action_ == A_NOTHING);


    // --------- on hover over --------------

    hoverCurveHash_ = Timeline1D::InvalidHash;

    auto oldHoverHash = hoverHash_;

    // get the timeline points in range
    auto it = tl_->first(x-rx),
         last = tl_->next_after(x+rx);
    // iterate over all of them
    for (; it != tl_->getData().end() && it != last; ++it)
    {
        if (x >= it->second.t - rx
         && x <= it->second.t + rx
         && y >= it->second.val - ry
         && y <= it->second.val + ry)
        {
            if (it->first == oldHoverHash)
                return;

            // set new hover point
            setHover_(it->second);

            setCursor(Qt::OpenHandCursor);

            e->accept();
            return;
        }
    }

    // unhover
    clearHover_();

    // ----- hover on curve -----

    it = tl_->closest(x);
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
                e->accept();
                hoverCurveHash_ = it->first;
                return;
            }
        }
    }


    // -- start dragging space --
    if (true /* can drag space */)
    {
        action_ = A_START_DRAG_SPACE;
        setCursor(Qt::SizeAllCursor);
    }
    else
        setCursor(Qt::ArrowCursor);
}

void Timeline1DView::mousePressEvent(QMouseEvent * e)
{
    if (!tl_)
        return;

    // ---- click on curve ----

    if (hoverCurveHash_ != Timeline1D::InvalidHash)
    {
        addPoint_(screen2time(e->x()), screen2value(e->y()));
        hoverCurveHash_ = Timeline1D::InvalidHash;
        e->accept();
        setCursor(Qt::OpenHandCursor);
        return;
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
            if (!(e->modifiers() & Qt::SHIFT)
                // or when already selected
                && !selectHashSet_.contains(hoverHash_))
                clearSelect_();

            addSelect_(point->second);
            action_ = A_DRAG_SELECTED;
            dragStart_ = e->pos();
            e->accept();
            setCursor(Qt::ClosedHandCursor);

            // copy the points to drag set
            dragPoints_.clear();
            for (auto &h : selectHashSet_)
            {
                auto it = tl_->getData().lower_bound(h);
                if (it != tl_->getData().end())
                    dragPoints_.push_back(DragPoint_(it));
            }
            return;
        }

        if (e->button() == Qt::RightButton)
        {
            // clear selection when clicking on unselected
            if (!(e->modifiers() & Qt::SHIFT)
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
        //if (!(e->modifiers() & Qt::SHIFT))
        //clearSelect_();

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
        return;
    }

    // --- double-click in empty space ---

    if (e->button() == Qt::LeftButton)
    {
        clearSelect_();
        e->accept();
        return;
    }
}

void Timeline1DView::mouseReleaseEvent(QMouseEvent * )
{
    action_ = A_NOTHING;
}


void Timeline1DView::selectAll_()
{
    selectHashSet_.clear();

    if (!tl_)
        return;

    for (auto &it : tl_->getData())
        selectHashSet_.insert(it.first);

    update();
}

void Timeline1DView::moveSelected_(Double dx, Double dy)
{
    if (!tl_)
        return;

    // only move vertically?
    if (fabs(dx) < Timeline1D::timeQuantum())
    {
        for (auto &p : dragPoints_)
        {
            if (p.valid)
                p.it->second.val = p.oldp.val + dy;
        }

        // TODO: only need to update the rect sourrounding the moved point
        update();
        return;
    }

    // potentially we change all the points so
    // we need to rebuild the selection hash
    selectHashSet_.clear();

    for (auto &p : dragPoints_)
    {
        if (!p.valid) continue;

        // create a new point
        auto newp = tl_->add(p.oldp.t + dx, p.oldp.val + dy, p.oldp.type);

        // was there a point already?
        if (newp == 0)
        {
            auto it2 = tl_->find(p.oldp.t + dx);

            // same point? then change value only
            if (it2 == p.it)
            {
                p.it->second.val = p.oldp.val + dy;
            }

            // update selection hash
            selectHashSet_.insert(p.it->first);
            continue;
        }

        // delete previous point
        tl_->getData().erase(p.it);
        p.valid = false;

        // keep the new point/iterator to find it next time
        p.newp = *newp;
        p.it = tl_->find(p.newp.t);

        if (p.it != tl_->getData().end())
        {
            p.valid = true;

            // update selection hash
            selectHashSet_.insert(p.it->first);
        }
    }

    update();
}

void Timeline1DView::slotPointContextMenu_()
{
    if (!isHover_() || !tl_)
        return;

    auto pointIt = tl_->getData().lower_bound(hoverHash_);
    if (pointIt == tl_->getData().end())
        return;

    QAction * a;

    // main popup
    QMenu * pop = new QMenu(this);
    connect(pop, SIGNAL(triggered(QAction*)), pop, SLOT(deleteLater()));

    // point type submenu
    QMenu * pointpop = new QMenu(pop);
    for (int i=1; i<Timeline1D::Point::MAX; ++i)
    {
        Timeline1D::Point::Type type = (Timeline1D::Point::Type)i;

        a = new QAction(Timeline1D::Point::getName(type), pointpop);
        pointpop->addAction(a);
        a->setCheckable(true);
        a->setChecked(pointIt->second.type == type);
        connect(a, &QAction::triggered, [=]()
        {
            changePointType_(type);
            update();
        });
    }

    // --- single point popup ---

    if (!isSelected_())
    {
        pop->addMenu(pointpop);
        pointpop->setTitle(tr("change point type"));

        a = new QAction(tr("delete point"), pop);
        pop->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            tl_->remove(pointIt->second.t);
            update();
        });
    }

    // --- selected points popup ---
    else
    {
        pop->addMenu(pointpop);
        pointpop->setTitle(tr("change points type"));

        a = new QAction(tr("delete points"), pop);
        pop->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            for (auto h : selectHashSet_)
                tl_->remove(h);
            update();
        });
    }


    pop->exec(QCursor::pos());
}

void Timeline1DView::slotEmptyContextMenu_()
{
    if (!tl_)
        return;

    QAction * a;

    // main popup
    QMenu * pop = new QMenu(this);
    connect(pop, SIGNAL(triggered(QAction*)), pop, SLOT(deleteLater()));

    a = new QAction(tr("select all"), pop);
    pop->addAction(a);
    connect(a, &QAction::triggered, [=]()
    {
        selectAll_();
    });

    a = new QAction(tr("clear selection"), pop);
    pop->addAction(a);
    connect(a, &QAction::triggered, [=]()
    {
        clearSelect_();
    });

    pop->addSeparator();

    a = new QAction(tr("fit to view"), pop);
    pop->addAction(a);
    connect(a, SIGNAL(triggered()), this, SLOT(fitToView()));

    a = new QAction(tr("fit selection to view"), pop);
    pop->addAction(a);
    connect(a, SIGNAL(triggered()), this, SLOT(fitSelectionToView()));

    pop->exec(QCursor::pos());
}

void Timeline1DView::changePointType_(Timeline1D::Point::Type t)
{
    if (isSelected_())
    {
        for (auto &h : selectHashSet_)
        {
            auto pointIt = tl_->getData().lower_bound(h);
            if (pointIt != tl_->getData().end())
                pointIt->second.type = t;
        }
    }
    else if (isHover_())
    {
        auto pointIt = tl_->getData().lower_bound(hoverHash_);
        if (pointIt != tl_->getData().end())
            pointIt->second.type = t;
    }
}

void Timeline1DView::addPoint_(Double t, Double v)
{
    if (!tl_)
        return;

    auto p = tl_->add(t, v);

    if (!p)
        return;

    setHover_(*p);
}

} // namespace GUI
} // namespace MO
