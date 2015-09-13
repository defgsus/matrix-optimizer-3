/** @file frontfloatitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.02.2015</p>
*/

#ifndef MO_DISABLE_FRONT

#ifndef MOSRC_GUI_ITEM_FRONTFLOATITEM_H
#define MOSRC_GUI_ITEM_FRONTFLOATITEM_H

#include "abstractfrontitem.h"
#include "types/float.h"

namespace MO {
namespace GUI {

/** Wraps all possible continous value float controls
*/
class FrontFloatItem : public AbstractFrontItem
{
public:

    // --------------- types --------------------

    enum ControlType
    {
        CT_FADER = 0,
        CT_KNOB
    };

    // ----------------- ctor -------------------

    explicit FrontFloatItem(QGraphicsItem * parent = 0);
    ~FrontFloatItem();

    // ---------------- getter ------------------

    /** Current ui value */
    Float value() const;

    Float defaultValue() const;

    // -------------- setter --------------------

    /** Sets the value of the ui item. No signals. */
    void setValue(Float v);

    // --------------- layout -------------------

    // ----------- virtual interface ------------
protected:

    const QString& className() const Q_DECL_OVERRIDE { static QString s("Float"); return s; }

    FrontFloatItem * cloneClass() const Q_DECL_OVERRIDE { return new FrontFloatItem; }

    int modulatorType() const Q_DECL_OVERRIDE;

    bool sendValue() Q_DECL_OVERRIDE;
    QVariant valueVariant() const Q_DECL_OVERRIDE { return QVariant(value()); }
    void setValueVariant(const QVariant&) Q_DECL_OVERRIDE;

    void onPropertiesChanged() Q_DECL_OVERRIDE;
    void onEditModeChanged() Q_DECL_OVERRIDE;

    // ----------- QGraphicsItem ----------------

    enum { Type = FIT_FLOAT };
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

#endif // MOSRC_GUI_ITEM_FRONTFLOATITEM_H

#endif // MO_DISABLE_FRONT
