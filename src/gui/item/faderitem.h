/** @file faderitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_FADERITEM_H
#define MOSRC_GUI_ITEM_FADERITEM_H

#include <functional>

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
    /* Length of value range in pixels */
    //int length() const;

    /** Returns the range (rangeMax() - rangeMin()).
        Value is clipped to lower range 0.000001 */
    Float range() const { return std::max(Float(0.000001), max_ - min_); }
    Float rangeMin() const { return min_; }
    Float rangeMax() const { return max_; }
    Float value() const { return value_; }
    Float defaultValue() const { return defValue_; }

    bool isControllable() const { return p_controllable_; }

    // --------------- setter -------------------

    void setOnColor(const QColor& c) { colorOn_ = c; update(); }
    void setOffColor(const QColor& c) { colorOff_ = c; update(); }
    void setVertical(bool v) { vertical_ = v; update(); }

    void setRange(Float mi, Float ma);
    void setDefaultValue(Float value) { defValue_ = value; update(); }
    void setValue(Float value);

    void setControllable(bool enable) { p_controllable_ = enable; }

    void setCallback(std::function<void(Float)> cb) { callback_ = cb; }

    // --------------- layout -------------------

    // ----------- QGraphicsItem ----------------

    enum { Type = GIT_FADER };
    int type() const { return Type; }

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QGraphicsSceneWheelEvent*) Q_DECL_OVERRIDE;
    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    void p_setEmit_(Float v);

    Float value_, defValue_, min_, max_,
        drag_start_value_;
    bool do_drag_;

    std::function<void(Float)> callback_;

    QColor colorOn_, colorOff_;
    bool vertical_, p_controllable_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FADERITEM_H
