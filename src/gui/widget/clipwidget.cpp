/** @file clipwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include <QPainter>
#include <QMouseEvent>
#include <QImage>

#include "clipwidget.h"
#include "clipwidgetbutton.h"
#include "gui/clipview.h"
#include "io/error.h"
#include "object/clip.h"

namespace MO {
namespace GUI {

ClipWidget::ClipWidget(Type type, Clip * clip, ClipView * parent)
    : QWidget       (parent),
      view_         (parent),
      clip_         (clip),
      type_         (type),
      button_       (0),
      hasFocus_     (false),
      x_            (0),
      y_            (0)
{
    setMouseTracking(true);

    setFixedSize(sizeForType(type_));

    button_ = new ClipWidgetButton(type_ != T_COLUMN
                                     ? ClipWidgetButton::T_PLAY
                                     : ClipWidgetButton::T_STOP, this);
    connect(button_, &ClipWidgetButton::clicked, [=]()
    {
        emit buttonClicked(this);
    });

    if (clip_)
    {
        name_ = clip_->name();
    }

    updateColors();
}

void ClipWidget::setClip(Clip * c)
{
    clip_ = c;

    if (clip_)
    {
        name_ = clip_->name();
    }
    else
        name_.clear();

    updateColors();

    update();
}

QSize ClipWidget::sizeForType(Type t)
{
    switch (t)
    {
        case T_COLUMN:  return QSize(60, 20);
        case T_ROW:     return QSize(60, 20);
        case T_CLIP:    return QSize(60, 20);
    }
    return QSize();
}

void ClipWidget::updateColors()
{
    button_->setVisible(type_ != T_CLIP || clip_ != 0);

    switch (type_)
    {
        case T_COLUMN:
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
            penText_ = QPen(Qt::white);
        break;

        case T_ROW:
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
            penText_ = QPen(Qt::white);
        break;

        case T_CLIP:
            penOutline_ = QPen(QColor(40,40,40));
            brushBody_ = clip_? clip_->color()
                              : QBrush(QColor(20,20,20));
            penText_ = brushBody_.color().lightness() > 128
                                ? QPen(Qt::black)
                                : QPen(Qt::white);
        break;
    }

    brushBodyH_ = brushBodyS_ = brushBodySH_ = brushBody_;
    brushBodyH_.setColor(brushBody_.color().lighter(110));
    brushBodyS_.setColor(brushBody_.color().lighter(120));
    brushBodySH_.setColor(brushBodyS_.color().lighter(110));

    penOutlineS_ = penOutline_;
    penOutlineS_.setColor(QColor(200,200,200));
    if (clip_)
        penOutlineS_.setWidth(penOutline_.width()+1);

    update();
}

void ClipWidget::resizeEvent(QResizeEvent *)
{
    button_->move(3, (height() - button_->height()) / 2);
}

void ClipWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // -- rect --

    if (view_->isSelected(this))
    {
        p.setPen(penOutlineS_);
        p.setBrush(hasFocus_? brushBodySH_ : brushBodyS_);
    }
    else
    {
        p.setPen(penOutline_);
        p.setBrush(hasFocus_? brushBodyH_ : brushBody_);
    }
    int b = penOutline_.width();
    p.drawRect(rect().adjusted(b/2,b/2,-b/2-1,-b/2-1));

    int x = b + 1;

    // -- playbutton --

    x += button_->width() + 1;

    // -- name --

    if (!name_.isEmpty())
    {
        if (nameText_.text() != name_)
            nameText_.setText(name_);

        p.setPen(penText_);
        p.drawStaticText(x + 1,
                         (height() - nameText_.size().height()) / 2,
                         nameText_);
    }
}


void ClipWidget::enterEvent(QEvent *)
{
    hasFocus_ = true;
    update();
}

void ClipWidget::leaveEvent(QEvent *)
{
    hasFocus_ = false;
    update();
}


void ClipWidget::mousePressEvent(QMouseEvent * e)
{
    emit clicked(this, e->buttons(), e->modifiers());
}

void ClipWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (!hasFocus_)
        return;

    emit moved(this, e->pos(), e->buttons(), e->modifiers());
}

void ClipWidget::mouseReleaseEvent(QMouseEvent * e)
{
    emit released(this, e->buttons(), e->modifiers());
}


void ClipWidget::setName(const QString & n)
{
    name_ = n;
    update();
}

void ClipWidget::setTriggered()
{
    if (!clip_)
        return;

    button_->setState(ClipWidgetButton::S_TRIGGERED);
}

void ClipWidget::setStopTriggered()
{
    if (!clip_)
        return;

    button_->setState(ClipWidgetButton::S_TRIGGERED);
}

void ClipWidget::setStarted()
{
    if (!clip_)
        return;

    button_->setState(ClipWidgetButton::S_ON);
}

void ClipWidget::setStopped()
{
    if (!clip_)
        return;

    button_->setState(ClipWidgetButton::S_OFF);
}


} // namespace GUI
} // namespace MO
