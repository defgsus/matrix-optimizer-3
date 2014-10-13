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
#include "object/param/modulatorfloat.h"
#include "io/error.h"

namespace MO {
namespace GUI {

namespace
{
    bool sortObjectList_Priority(const Object * o1, const Object * o2)
    {
        return Object::objectPriority(o1) > Object::objectPriority(o2);
    }
}

ObjectMenu::ObjectMenu()
{
}

QMenu * ObjectMenu::createObjectMenu(int objectTypeFlags, QWidget *parent)
{
    QList<const Object*> list(ObjectFactory::objects(objectTypeFlags));
    if (list.empty())
        return 0;

    // sort by priority
    qStableSort(list.begin(), list.end(), sortObjectList_Priority);

    QMenu * menu = new QMenu(parent);
    int curprio = Object::objectPriority( list.front() );
    for (auto o : list)
    {
        if (curprio != Object::objectPriority(o))
        {
            menu->addSeparator();
            curprio = Object::objectPriority(o);
        }

        QAction * a = new QAction(ObjectFactory::iconForObject(o), o->name(), parent);
        a->setData(o->className());
        menu->addAction(a);
    }

    return menu;
}



QMenu * ObjectMenu::createObjectChildMenu(Object *root, int objectTypeFlags, QWidget *parent)
{
    QMenu * menu = new QMenu(parent);

    createObjectMenuRecursive_(menu, root, objectTypeFlags);

    return menu;
}

void ObjectMenu::createObjectMenuRecursive_(QMenu * menu, Object *root, int objectTypeFlags)
{
    // add all children ...
    for (auto c : root->childObjects())
    {
        const QList<Object*> list =
            c->findChildObjects(objectTypeFlags, true);

        const bool match = (c->type() & objectTypeFlags);

        // ... when they themself or one of their childs
        // are of the requested type
        if (list.isEmpty() && !match)
            continue;

        QAction * a = new QAction(ObjectFactory::iconForObject(c), c->name(), menu);
        if (match)
            a->setData(c->idName());

        // add sub menu
        if (c->numChildren())
        {
            QMenu * m = new QMenu(menu);

            createObjectMenuRecursive_(m, c, objectTypeFlags);
            if (m->isEmpty())
                m->deleteLater();
            else
            {
                // create a new action for the submenu if
                // the previous action should be a clickable object
                if (match)
                {
                    menu->addAction(a);
                    a = new QAction(ObjectFactory::iconForObject(c), c->name(), menu);
                    a->setData(c->idName());
                }
                a->setMenu(m);
            }
        }

       a->setEnabled( match || a->menu());

       menu->addAction(a);
    }
}


QMenu * ObjectMenu::createRemoveModulationMenu(Parameter * param, QWidget *parent)
{
    //static QIcon iconTrack(QIcon(":/icon/obj_track.png"));

    QMenu * menu = new QMenu(parent);

    if (ParameterFloat * pf = dynamic_cast<ParameterFloat*>(param))
    {
        for (auto m : pf->modulators())
        {
            Object * mo = m->modulator();
            MO_ASSERT(mo, "no object assigned to modulator of '" << pf->idName() << "'");
            QAction * a = new QAction(ObjectFactory::iconForObject(mo), mo->name(), menu);
            a->setData(mo->idName());
            a->setToolTip(mo->namePath());
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



void ObjectMenu::setEnabled(QMenu *menu, const QStringList& ids, bool enable)
{
    for (QAction * a : menu->actions())
    {
        if (ids.contains(a->data().toString()))
            a->setEnabled(enable);

        // go through child menus
        if (a->menu())
            setEnabled(a->menu(), ids, enable);
    }
}



} // namespace GUI
} // namespace MO
