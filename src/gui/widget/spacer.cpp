/** @file spacer.cpp

    @brief Drag/spacer bar between two widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/8/2014</p>
*/
#include <QDebug>
#include <QMouseEvent>

#include "spacer.h"

namespace MO {
namespace GUI {

Spacer::Spacer(Qt::Orientation o, QWidget *parent) :
    QWidget (parent),
    left_   (0),
    right_  (0),
    width_  (3),
    adjustLeft_(true),
    dragging_(false)
{
    setOrientation(o);

    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, QColor(90,90,90));
    setPalette(p);
}

void Spacer::setOrientation(Qt::Orientation o)
{
    orientation_ = o;

    if (o == Qt::Vertical)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        setFixedWidth(width_);
        setMinimumHeight(10);
        setMaximumHeight(QWIDGETSIZE_MAX);
        setCursor(Qt::SplitHCursor);
    }
    else
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        setFixedHeight(width_);
        setMinimumWidth(10);
        setMaximumWidth(QWIDGETSIZE_MAX);
        setCursor(Qt::SplitVCursor);
    }
}

void Spacer::setSpacerWidth(int w)
{
    width_ = w;
    setOrientation(orientation_);
}

void Spacer::setWidgets(QWidget *left, QWidget *right, bool adjustLeft)
{
    left_ = left;
    right_ = right;
    adjustLeft_ = adjustLeft;

    QWidget * which = adjustLeft_? left_ : right_;
    minSize_ = orientation_ == Qt::Vertical? which->minimumWidth() : which->minimumHeight();
    if (orientation_ == Qt::Vertical)
        which->setFixedWidth(minSize_);
    else
        which->setFixedHeight(minSize_);
}


void Spacer::mousePressEvent(QMouseEvent * e)
{
    if (!left_ || !right_)
        return;

    if (e->button() == Qt::LeftButton)
    {
        dragging_ = true;
        dragStart_ = mapToGlobal(e->pos());
        dragStartRect_ = adjustLeft_? left_->rect() : right_->rect();

        e->accept();
    }
}

void Spacer::mouseMoveEvent(QMouseEvent * e)
{
    if (dragging_)
    {
        const QPoint g = mapToGlobal(e->pos());

        const int delta = (orientation_ == Qt::Vertical)
                ? g.x() - dragStart_.x()
                : g.y() - dragStart_.y();

        if (!delta)
            return;

        if (adjustLeft_)
        {
            if (orientation_ == Qt::Vertical)
                left_->setFixedWidth(std::max(minSize_, dragStartRect_.width() + delta));
            else
                left_->setFixedHeight(std::max(minSize_, dragStartRect_.height() + delta));
        }
        else
        {
            if (orientation_ == Qt::Vertical)
                right_->setFixedWidth(std::max(minSize_, dragStartRect_.width() - delta));
            else
                right_->setFixedHeight(std::max(minSize_, dragStartRect_.height() - delta));
        }

        emit dragged();

        e->accept();
    }
}

void Spacer::mouseReleaseEvent(QMouseEvent * )
{
    dragging_ = false;
}



} // namespace GUI
} // namespace MO
