/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/4/2016</p>
*/

#ifndef MOSRC_GUI_ITEM_OBJECTGRAPHCONTROLITEM_H
#define MOSRC_GUI_ITEM_OBJECTGRAPHCONTROLITEM_H

#include <functional>
#include <QGraphicsPixmapItem>

namespace MO {
namespace GUI {

class AbstractObjectItem;

/** An clickable item in the corner of an AbstractObjectItem.
    Typically controls activity and visibility */
class ObjectGraphControlItem : public QGraphicsPixmapItem
{
public:

    enum { Type = UserType + 1 };

    ObjectGraphControlItem(AbstractObjectItem * parent);

    void setCallback(std::function<void()> f) { func_ = f; }

    // ---------- QGraphicsItem interface --------------

    virtual int type() const Q_DECL_OVERRIDE { return Type; }

    void paint(QPainter * p, const QStyleOptionGraphicsItem *, QWidget *) Q_DECL_OVERRIDE;

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent*);

private:

    AbstractObjectItem * objectItem_;
    std::function<void()> func_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_OBJECTGRAPHCONTROLITEM_H
