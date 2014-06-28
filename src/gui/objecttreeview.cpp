/** @file objecttreeeditor.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#include "objecttreeview.h"

namespace MO {
namespace GUI {


ObjectTreeView::ObjectTreeView(QWidget *parent) :
    QTreeView(parent)
{
    setDragEnabled(true);
    setDragDropMode(DragDrop);
}


} // namespace GUI
} // namespace MO
