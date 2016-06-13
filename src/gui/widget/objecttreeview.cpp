/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#include "objecttreeview.h"

namespace MO {
namespace GUI {

ObjectTreeView::ObjectTreeView(QWidget *parent)
    : QTreeView     (parent)
{
    setHeaderHidden(true);
    //setColumnHidden(1, true);
}


} // namespace GUI
} // namespace MO
