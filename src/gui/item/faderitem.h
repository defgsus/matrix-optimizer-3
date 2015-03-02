/** @file faderitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_FADERITEM_H
#define MOSRC_GUI_ITEM_FADERITEM_H

#include "abstractguiitem.h"
#include "types/float.h"

namespace MO {
namespace GUI {

/**
*/
class FaderItem : public AbstractGuiItem
{
public:

    FaderItem(QGraphicsItem * parent = 0);

    // ---------------- getter ------------------

    const QColor onColor() const { return colorOn_; }
    const QColor offColor() const { return colorOff_; }
    bool vertical() const { return vertical_; }

    // --------------- setter -------------------

    void setOnColor(const QColor& c) { colorOn_ = c; update(); }
    void setOffColor(const QColor& c) { colorOff_ = c; update(); }
    void setVertical(bool v) { vertical_ = v; update(); }

    // --------------- layout -------------------

    // ----------- QGraphicsItem ----------------

    enum { Type = Qt::UserRole + 2048 };
    int type() const { return Type; }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    Float value_, min_, max_,
        drag_start_value_;
    bool do_drag_;

    QColor colorOn_, colorOff_;
    bool vertical_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FADERITEM_H
