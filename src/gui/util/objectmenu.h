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
        The QAction::data() contains the Object::idName() and Modulator::outputId()
        as special format. Use getModulatorId() and getOutputId() to convert. */
    static QMenu * createRemoveModulationMenu(Parameter *, QWidget * parent = 0);

    static QString getModulatorId(const QString& modAndOutputId);
    static QString getOutputId(const QString& modAndOutputId);

    /** Sets the enabled state of each menu action matching one of the @p ids to
        @p enable */
    static void setEnabled(QMenu * menu, const QStringList& ids, bool enable);

    /** Creates a menu with all parameters of the object in their groups.
        The QAction::data() contains the Parameter::idName(). */
    static QMenu * createParameterMenu(Object * o, QWidget * parent = 0);

    /** Creates a menu with all parameters of the object in their groups.
        @p selector enables or disables certain parameters.
        The QAction::data() contains the Parameter::idName().
        @note A NULL pointer is returned if none of the parameters satisfied the selector. */
    static QMenu * createParameterMenu(Object * o, QWidget * parent,
                                       std::function<bool(Parameter*)> selector);


    /** Creates a menu to select a color.
        The QColor is stored in QAction::data() */
    static QMenu * createColorMenu(QWidget * parent = 0);

    /** Creates a menu to select a color.
        The hue int is stored in QAction::data().
        -1 is considered gray. */
    static QMenu * createHueMenu(QWidget * parent = 0);

private:

    static void createObjectMenuRecursive_(QMenu *, Object * root, int objectTypeFlags);

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTMENU_H
