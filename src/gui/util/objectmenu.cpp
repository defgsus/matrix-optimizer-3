/** @file objectmenu.cpp

    @brief Creator for specifc popup menus

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QIcon>
#include <QMenu>
#include <QAction>

#include "objectmenu.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/param/parameter.h"
#include "object/param/parameterfloat.h"
#include "object/trackfloat.h"

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
        const QList<Object*> list =
            c->findChildObjects(objectTypeFlags, true);

        const bool match = (c->type() & objectTypeFlags);

        if (list.isEmpty() && !match)
            continue;

        QAction * a = new QAction(ObjectFactory::iconForObject(c), c->name(), menu);
        if (match)
            a->setData(c->idName());

        if (c->numChildren())
        {
            QMenu * m = new QMenu(menu);
            createObjectMenuRecursive_(m, c, objectTypeFlags);
            if (m->isEmpty())
                m->deleteLater();
            else
                a->setMenu(m);
        }

       a->setEnabled( match || a->menu());

       menu->addAction(a);
    }
}


QMenu * ObjectMenu::createRemoveModulationMenu(Parameter * param, QWidget *parent)
{
    QIcon iconTrack(QIcon(":/icon/obj_track.png"));

    QMenu * menu = new QMenu(parent);

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param))
    {
        for (auto m : pf->modulators())
        {
            QAction * a = new QAction(iconTrack, m->name(), menu);
            a->setData(m->idName());
            a->setToolTip(m->namePath());
            a->setStatusTip(a->toolTip());
            menu->addAction(a);
        }
    }
    else
    {
        for (auto id : param->modulatorIds())
        {
            QAction * a = new QAction(id, menu);
            a->setData(id);
            menu->addAction(a);
        }
    }

    return menu;
}


} // namespace GUI
} // namespace MO
