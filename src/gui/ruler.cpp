/** @file ruler.cpp

    @brief basic adjustable grid view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

#include "ruler.h"
#include "gui/painter/grid.h"

namespace MO {
namespace GUI {


Ruler::Ruler(QWidget *parent) :
    QWidget         (parent),
    gridPainter_    (new PAINTER::Grid(this)),
    options_        (O_EnableAll),
    action_         (A_NOTHING)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    setMouseTracking(true);

    // update grid painter
    setOptions(options_);
}

void Ruler::setViewSpace(const UTIL::ViewSpace & v, bool send_signal)
{
    space_ = v;
    if (send_signal)
        emit viewSpaceChanged(space_);
    update();
}

void Ruler::setOptions(int options)
{
    bool changed = (options_ != options);

    options_ = options;

    gridPainter_->setOptions(
        (PAINTER::Grid::O_DrawX * ((options_ & O_DrawX) != 0))
    |   (PAINTER::Grid::O_DrawY * ((options_ & O_DrawY) != 0))
    |   (PAINTER::Grid::O_DrawTextX * ((options_ & O_DrawTextX) != 0))
    |   (PAINTER::Grid::O_DrawTextY * ((options_ & O_DrawTextY) != 0))
    );

    setCursor(defaultCursor_());

    if (changed)
        update();
}

void Ruler::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // -- background --

    p.setPen(Qt::NoPen);
    p.setBrush(QBrush(QColor(50,50,50)));
    p.drawRect(e->rect());

    // -- grid --

    gridPainter_->setViewSpace(space_);
    gridPainter_->paint(p, e->rect());

}


void Ruler::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton
        && (options() & O_DragAll))
    {
        dragStart_ = lastPos_ = e->pos();
        dragStartSpace_ = space_;
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        action_ = A_DRAG_SPACE;
        return;
    }

    if (e->button() == Qt::LeftButton)
    {
        emit fitRequest();
        e->accept();
        return;
    }
}

void Ruler::mouseMoveEvent(QMouseEvent * e)
{
    const Double zoomChange_ = 0.01;

    if (action_ == A_DRAG_SPACE)
    {
        bool changed = false;

        Double
            dx = space_.mapXDistanceTo(lastPos_.x() - e->x()) / width(),
            dy = space_.mapYDistanceTo(lastPos_.y() - e->y()) / height();

        if (options_ & O_DragX)
        {
            space_.addX( dx );
            changed = true;
        }
        else if (options_ & O_ZoomY)
        {
            space_.zoomY( 1.0 + zoomChange_ * (lastPos_.x() - e->x()),
                          (Double)e->y() / height() );
            changed = true;
        }

        if (options_ & O_DragY)
        {
            space_.addY( -dy );
            changed = true;
        }
        else if (options_ & O_ZoomX)
        {
            space_.zoomX( 1.0 + zoomChange_ * (lastPos_.y() - e->y()),
                          (Double)e->x() / width() );
            changed = true;
        }

        if (changed)
        {
            emit viewSpaceChanged(space_);
            update();
        }

        lastPos_ = e->pos();

        e->accept();
        return;
    }
}

void Ruler::mouseReleaseEvent(QMouseEvent * )
{
    setCursor(defaultCursor_());
    action_ = A_NOTHING;
}

void Ruler::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == Qt::LeftButton)
    {
        const Double time =
                    (options_ & O_EnableAllX)?
                        space_.mapXTo((Double)e->x()/width())
                      : space_.mapYTo((Double)e->y()/height());
        emit doubleClicked(time);
        e->accept();
        return;
    }
}

QCursor Ruler::defaultCursor_() const
{
    if (options_ & O_ChangeViewAll)
        return Qt::OpenHandCursor;
    else
        return Qt::ArrowCursor;
}


} // namespace GUI
} // namespace MO
