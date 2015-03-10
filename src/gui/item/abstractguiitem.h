/** @file abstractguiitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef ABSTRACTGUIITEM_H
#define ABSTRACTGUIITEM_H

#include <QGraphicsItem>

namespace MO {
namespace GUI {

/** Very general (normally rectangular) interface item,
    connecting some value/object with some gui control.
*/
class AbstractGuiItem : public QGraphicsItem
{
public:

    AbstractGuiItem(QGraphicsItem * parent = 0);

    // ---------------- getter ------------------

    QSizeF size() const { return p_rect_.size(); }
    qreal width() const { return p_rect_.width(); }
    qreal height() const { return p_rect_.height(); }
    QRectF rect() const { return p_rect_; }

    // --------------- setter -------------------

    void setSize(const QSizeF& s) { p_rect_.setSize(s); }
    void setRect(const QRectF& r) { p_rect_ = r; }

    // ----------- QGraphicsItem ----------------

    enum { Type = UserType + 2048 };
    int type() const { return Type; }

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE { return p_rect_; }

    //virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    QRectF p_rect_;
};

} // namespace GUI
} // namespace MO

#endif // ABSTRACTGUIITEM_H
