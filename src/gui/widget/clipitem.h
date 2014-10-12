/** @file clipitem.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_CLIPITEM_H
#define MOSRC_GUI_WIDGET_CLIPITEM_H

#include <QGraphicsItem>

namespace MO {
namespace GUI {

class ClipItem : public QGraphicsItem
{
public:
    explicit ClipItem(QGraphicsItem *parent = 0);

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_CLIPITEM_H
