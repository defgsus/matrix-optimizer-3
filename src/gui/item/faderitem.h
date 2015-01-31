/** @file faderitem.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_ITEM_FADERITEM_H
#define MOSRC_GUI_ITEM_FADERITEM_H

#include "abstractguiitem.h"

namespace MO {
namespace GUI {

/**
*/
class FaderItem : public AbstractGuiItem
{
public:

    FaderItem(QGraphicsItem * parent = 0);

    // ---------------- getter ------------------

    // --------------- setter -------------------

    // --------------- layout -------------------

    // ----------- QGraphicsItem ----------------

    enum { Type = Qt::UserRole + 2048 };
    int type() const { return Type; }

    virtual void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

private:

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_FADERITEM_H
