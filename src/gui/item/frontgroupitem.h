/** @file frontgroupitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.02.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_FRONTGROUPITEM_H
#define MOSRC_GUI_ITEM_FRONTGROUPITEM_H

#include "abstractfrontitem.h"

namespace MO {
namespace GUI {

/** A simple group.
    This is just the non-abstract form of AbstractFrontItem.
*/
class FrontGroupItem : public AbstractFrontItem
{
public:

    explicit FrontGroupItem(QGraphicsItem * parent = 0);
    ~FrontGroupItem();

    // ---------------- getter ------------------

    // -------------- setter --------------------

    // --------------- layout -------------------

    // ----------- virtual interface ------------
protected:
    const QString& className() const Q_DECL_OVERRIDE { static QString s("Group"); return s; }

    FrontGroupItem * cloneClass() const Q_DECL_OVERRIDE { return new FrontGroupItem; }

    void onPropertiesChanged() Q_DECL_OVERRIDE;
    //void onEditModeChanged() Q_DECL_OVERRIDE;
public:

    // ----------- QGraphicsItem ----------------

    enum { Type = FIT_GROUP };
    virtual int type() const { return Type; }

private:

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FRONTGROUPITEM_H
