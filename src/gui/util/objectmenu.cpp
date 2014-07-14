/** @file objectmenu.cpp

    @brief Creator for specifc popup menus

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QMenu>
#include <QAction>

#include "objectmenu.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/param/parameter.h"

namespace MO {
namespace GUI {


ObjectMenu::ObjectMenu()
{
}

QMenu * ObjectMenu::createObjectMenu(Object *root, int objectTypeFlags, QWidget *parent)
{
    QMenu * menu = new QMenu(parent);

    createObjectMenuRecursive_(menu, root, objectTypeFlags);

    return menu;
}


void ObjectMenu::createObjectMenuRecursive_(QMenu * menu, Object *root, int objectTypeFlags)
{
    // add all children

    for (auto c : root->childObjects())
    {
        QAction * a = new QAction(ObjectFactory::iconForObject(c), c->name(), menu);
        //a->setEnabled(c->type() & objectTypeFlags);
        menu->addAction(a);

        if (c->numChildren())
        {
            QMenu * m = new QMenu(menu);
            createObjectMenuRecursive_(m, c, objectTypeFlags);
            a->setMenu(m);
        }
    }
}


QMenu * ObjectMenu::createRemoveModulationMenu(Parameter * param, QWidget *parent)
{
    QMenu * menu = new QMenu(parent);

    for (auto id : param->modulatorIds())
    {
        QAction * a = new QAction(id, menu);
        a->setData(id);
        menu->addAction(a);
    }

    return menu;
}


} // namespace GUI
} // namespace MO
