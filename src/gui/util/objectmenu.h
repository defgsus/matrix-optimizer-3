/** @file objectmenu.h

    @brief Creator for specifc popup menus

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTMENU_H
#define MOSRC_GUI_UTIL_OBJECTMENU_H

#include <QStringList>

#include "object/object_fwd.h"

class QWidget;
class QMenu;

namespace MO {
namespace GUI {

class ObjectMenu
{
public:
    ObjectMenu();

    /** Creates a menu with all objects matching the types.
        QAction::data() contains the Object::className().
        Returns NULL when typeflags did not match any object! */
    static QMenu * createObjectMenu(int objectTypeFlags, QWidget * parent = 0);

    /** Creates a menu with all objects in @p root matching @p objectTypeFlags.
        The QAction::data() contains the Object::idName(). */
    static QMenu * createObjectChildMenu(Object * root, int objectTypeFlags, QWidget *parent = 0);

    /** Creates a menu with all modulators for the given Parameter.
        The QAction::data() contains the Object::idName(). */
    static QMenu * createRemoveModulationMenu(Parameter *, QWidget * parent = 0);

    /** Sets the enabled state of each menu action matching one of the @p ids to
        @p enable */
    static void setEnabled(QMenu * menu, const QStringList& ids, bool enable);

private:

    static void createObjectMenuRecursive_(QMenu *, Object * root, int objectTypeFlags);

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTMENU_H
