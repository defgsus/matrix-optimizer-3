/** @file scopeitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11.03.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_SCOPEITEM_H
#define MOSRC_GUI_ITEM_SCOPEITEM_H

#include <functional>

#include "AbstractGuiItem.h"
#include "types/float.h"

namespace MO {
namespace GUI {
namespace PAINTER { class ValueCurve; }
/**
*/
class ScopeItem : public AbstractGuiItem
{
public:

    ScopeItem(QGraphicsItem * parent = 0);
    ~ScopeItem();

    // ---------------- getter ------------------

    const QColor color() const { return color_; }
    const QColor backgroundColor() const { return colorBack_; }

    /** Returns the range (rangeMax() - rangeMin()).
        Value is clipped to lower range 0.000001 */
    Float range() const { return std::max(Float(0.000001), max_ - min_); }
    Float rangeMin() const { return min_; }
    Float rangeMax() const { return max_; }

    // --------------- setter -------------------

    void setColor(const QColor& c) { color_ = c; update(); }
    void setBackgroundColor(const QColor& c) { colorBack_ = c; update(); }

    void setRange(Float mi, Float ma);
    void setValue(Float value);

    // --------------- layout -------------------

    // ----------- QGraphicsItem ----------------

    enum { Type = GIT_SCOPE };
    int type() const { return Type; }

//    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
//    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
//    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
//    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
//    virtual void wheelEvent(QGraphicsSceneWheelEvent*) Q_DECL_OVERRIDE;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    Float min_, max_;

    QColor color_, colorBack_;
    PAINTER::ValueCurve * draw_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_ITEM_SCOPEITEM_H
