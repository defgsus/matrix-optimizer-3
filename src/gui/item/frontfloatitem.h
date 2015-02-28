/** @file frontfloatitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.02.2015</p>
*/

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

    explicit FrontFloatItem(QGraphicsItem * parent = 0);
    ~FrontFloatItem();

    // ---------------- getter ------------------

    Float value() const;

    // -------------- setter --------------------

    void setValue(Float v);

    // --------------- layout -------------------

    // ----------- virtual interface ------------
protected:

    const QString& className() const { static QString s("Float"); return s; }

    FrontFloatItem * cloneClass() const { return new FrontFloatItem; }

    void onPropertiesChanged() Q_DECL_OVERRIDE;
    void onEditModeChanged() Q_DECL_OVERRIDE;

    // ----------- QGraphicsItem ----------------

    enum { Type = FIT_FLOAT };
    virtual int type() const { return Type; }

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FRONTFLOATITEM_H
