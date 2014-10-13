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
      hasFocus_     (false),
      isSelected_   (false)
{
    setMouseTracking(true);

    if (clip_)
        name_ = clip_->name();

    updateColors_();
}

void ClipWidget::setClip(Clip * c)
{
    if (clip_ == c)
        return;

    clip_ = c;

    if (clip_)
        name_ = clip_->name();

    updateColors_();

    update();
}

void ClipWidget::updateColors_()
{
    switch (type_)
    {
        case T_COLUMN:
            setFixedSize(60, 20);
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
            penText_ = QPen(Qt::white);
        break;

        case T_ROW:
            setFixedSize(20, 20);
            penOutline_ = Qt::NoPen;
            brushBody_ = QBrush(QColor(100,100,100));
            penText_ = QPen(Qt::white);
        break;

        case T_CLIP:
            setFixedSize(60, 20);
            penOutline_ = QPen(QColor(40,40,40));
            brushBody_ = clip_? QBrush(QColor(130,170,130))
                              : QBrush(QColor(20,20,20));
            penText_ = QPen(Qt::white);
        break;
    }

    brushBodyH_ = brushBodyS_ = brushBodySH_ = brushBody_;
    brushBodyH_.setColor(brushBody_.color().lighter(120));
    brushBodyS_.setColor(brushBody_.color().lighter(150));
    brushBodySH_.setColor(brushBodyS_.color().lighter(120));

    penOutlineS_ = penOutline_;
    penOutlineS_.setColor(penOutline_.color().lighter(180));
    penOutlineS_.setWidth(penOutline_.width()+1);
}

void ClipWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // -- rect --

    if (isSelected_)
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

    if (type_ == T_CLIP)
    {
        static const QImage play_off(":/icon/play_off_small");
        static const QImage play_on(":/icon/play_on_small");
        static const QImage stop(":/icon/stop_small");

        int y = (height()-play_off.height())/2;
        if (!clip_)
            p.drawImage(x, y, stop);
        else
            p.drawImage(x, y, clip_->isPlaying() ? play_on : play_off);

        x += play_off.width() + 1;
    }

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
    //if (e->buttons() & Qt::LeftButton)
    emit clicked(this, e->buttons(), e->modifiers());
}

void ClipWidget::mouseMoveEvent(QMouseEvent * )
{
    if (!hasFocus_)
        return;
}

void ClipWidget::mouseReleaseEvent(QMouseEvent * )
{

}


void ClipWidget::setName(const QString & n)
{
    name_ = n;
    update();
}


void ClipWidget::setSelected(bool s)
{
    if (isSelected_ == s)
        return;

    isSelected_ = s;

    update();
}


} // namespace GUI
} // namespace MO
