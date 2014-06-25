/** @file ruler.cpp

    @brief basic adjustable grid view

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

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

    setCursor(Qt::OpenHandCursor);
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
        dragStart_ = e->pos();
        dragStartSpace_ = space_;
        setCursor(Qt::ClosedHandCursor);
        e->accept();
        action_ = A_DRAG_SPACE;
        return;
    }
}

void Ruler::mouseMoveEvent(QMouseEvent * e)
{
    if (action_ == A_DRAG_SPACE)
    {
        Double
            dx = space_.mapXDistanceTo(e->x() - dragStart_.x()) / width(),
            dy = space_.mapYDistanceTo(e->y() - dragStart_.y()) / height();

        if (!(options_ & O_DragX))
            dx = 0;
        if (!(options_ & O_DragY))
            dy = 0;

        if (dx || dy)
        {
            space_.setX( dragStartSpace_.x() - dx );
            space_.setY( dragStartSpace_.y() + dy );
            emit viewSpaceChanged(space_);
            update();
        }

        e->accept();
        return;
    }
}

void Ruler::mouseReleaseEvent(QMouseEvent * )
{
    setCursor(Qt::OpenHandCursor);
    action_ = A_NOTHING;
}



} // namespace GUI
} // namespace MO
