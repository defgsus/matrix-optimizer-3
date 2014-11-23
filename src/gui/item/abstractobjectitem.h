/** @file abstractobjectitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H
#define MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H

#include <QGraphicsItem>

namespace MO {
class Object;
namespace GUI {

class AbstractObjectItem : public QGraphicsItem
{
public:
    AbstractObjectItem(Object * object, QGraphicsItem * parent = 0);
    ~AbstractObjectItem();

    // -------------------- getter ---------------------

    /** Returns the assigned object */
    Object * object() const;

    /** Returns the icon for the assigned object */
    const QIcon & icon() const;

private:

    class PrivateOI;
    PrivateOI * p_oi_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_ITEM_ABSTRACTOBJECTITEM_H
