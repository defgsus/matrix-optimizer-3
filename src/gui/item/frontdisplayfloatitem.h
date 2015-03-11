/** @file frontdisplayfloatitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.03.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_FRONTDISPLAYFLOATITEM_H
#define MOSRC_GUI_ITEM_FRONTDISPLAYFLOATITEM_H

#include "abstractfrontdisplayitem.h"
#include "types/float.h"

namespace MO {
namespace GUI {

/** Wraps all possible continous value float displays.
*/
class FrontDisplayFloatItem : public AbstractFrontDisplayItem
{
public:

    // --------------- types --------------------

    enum DisplayType
    {
        DT_FADER = 0,
        DT_SCOPE
    };

    // ----------------- ctor -------------------

    explicit FrontDisplayFloatItem(QGraphicsItem * parent = 0);
    ~FrontDisplayFloatItem();

    // ---------------- getter ------------------

    /** Returns the current float value, if an Object is assigned. */
    Float value() const;

    // -------------- setter --------------------

    // --------------- layout -------------------

    // ----------- virtual interface ------------
protected:

    const QString& className() const Q_DECL_OVERRIDE { static QString s("FloatDisplay"); return s; }

    FrontDisplayFloatItem * cloneClass() const Q_DECL_OVERRIDE { return new FrontDisplayFloatItem; }

    void onPropertiesChanged() Q_DECL_OVERRIDE;
    void onEditModeChanged() Q_DECL_OVERRIDE;

    bool acceptObject(Object*) const Q_DECL_OVERRIDE;

    void updateValue() Q_DECL_OVERRIDE;

    // ----------- QGraphicsItem ----------------

    enum { Type = FIT_DISPLAY_FLOAT };
    virtual int type() const { return Type; }

    //virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*) Q_DECL_OVERRIDE;

    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FRONTDISPLAYFLOATITEM_H
