/** @file objectmenu.h

    @brief Creator for specifc popup menus

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTMENU_H
#define MOSRC_GUI_UTIL_OBJECTMENU_H

#include "object/object_fwd.h"

class QWidget;
class QMenu;

namespace MO {
namespace GUI {

class ObjectMenu
{
public:
    ObjectMenu();

    /** Creates a menu with all objects matching @p objectTypeFlags.
        The QAction::data() contains the Object::idName(). */
    static QMenu * createObjectMenu(Object * root, int objectTypeFlags, QWidget *parent = 0);

    static QMenu * createRemoveModulationMenu(Parameter *, QWidget * parent = 0);

private:

    static void createObjectMenuRecursive_(QMenu *, Object * root, int objectTypeFlags);

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTMENU_H
