/** @file objectmenu.cpp

    @brief Creator for specifc popup menus

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/14/2014</p>
*/

#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QPainter>

#include "objectmenu.h"
#include "object/object.h"
#include "object/objectfactory.h"
#include "object/param/parameter.h"
#include "object/param/parameters.h"
#include "object/param/parameterfloat.h"
#include "object/param/modulatorfloat.h"
#include "object/trackfloat.h"
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
            // YYY no outputId
            QAction * a = new QAction(id.first, menu);
            a->setData(id.first);
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

QMenu * ObjectMenu::createParameterMenu(Object *o, QWidget *parent)
{
    return createParameterMenu(o, parent, [](Parameter*){ return true; } );
}

QMenu * ObjectMenu::createParameterMenu(Object *o, QWidget *parent,
                                        std::function<bool(Parameter*)> selector)
{
    QMenu * menu = new QMenu(parent);

    // sort into groups
    QMultiMap<QString, Parameter*> params;

    for (Parameter * p : o->params()->parameters())
        params.insert(p->groupId(), p);

    QMenu * sub = menu;

    QString curId = "-1";
    for (auto i = params.begin(); i != params.end(); ++i)
    {
        // create new group
        if (curId != i.key())
        {
            curId = i.key();

            if (!curId.isEmpty())
            {
                sub = new QMenu(curId, menu);
                menu->addMenu(sub);
            }
        }

        QAction * a = new QAction(i.value()->name(), menu);
        a->setData(i.value()->idName());
        a->setEnabled(selector(i.value()));
        sub->addAction(a);
    }

    return menu;
}




QMenu * ObjectMenu::createColorMenu(QWidget *parent)
{
    QMenu * menu = new QMenu(parent);

    // --- create color list ---

    static QList<QColor> base_colors;
    if (base_colors.isEmpty())
    {
        base_colors
                << QColor(255, 255, 255);

        for (int i=0; i < 360; i += 30)
            base_colors
                << QColor::fromHsv(i, 255, 128);
    }

    // create dark-bright gradient of base_colors
    static QList<QList<QColor>> colors;
    if (colors.empty())
    {
        for (auto & c : base_colors)
        {
            QList<QColor> cols;

            const int brightest = (c == base_colors.front())
                    ? 255 : 235;
            for (int i=brightest; i>=20; i -= 20)
                cols << QColor::fromHsl(c.hue(), c.saturation(), i);

            colors << cols;
        }
    }

    // --- create icon lists ---

    static QList<QList<QIcon>> icons;
    if (icons.isEmpty())
    {
        for (auto & cols : colors)
        {
            QList<QIcon> list;

            for (auto & col : cols)
            {
                QPixmap pixmap(32, 32);
                QPainter painter(&pixmap);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(col));
                painter.drawRect(pixmap.rect());

                list << QIcon(pixmap);
            }
            icons << list;
        }
    }

    for (int i=0; i<icons.size(); ++i)
    {
        QAction * a = new QAction(icons[i][icons[i].size()/2], "", menu);
        QMenu * sub = new QMenu(menu);
        a->setMenu(sub);
        menu->addAction(a);
        for (int j=0; j<icons[i].size(); ++j)
        {
            QAction * a = new QAction(icons[i][j], "", sub);
            a->setData(colors[i][j]);
            sub->addAction(a);
        }
    }

    return menu;
}


QMenu * ObjectMenu::createHueMenu(QWidget *parent)
{
    QMenu * menu = new QMenu(parent);

    // --- create icon lists ---

    static QList<QIcon> icons;
    static QList<int> hues;
    if (icons.isEmpty())
    {
        for (int i = -23; i < 359; i += 23)
        {
            QColor col = i < 0 ? QColor::fromHsl(0, 0, 100)
                               : QColor::fromHsl(i, 200, 100);

            QPixmap pixmap(32, 32);
            QPainter painter(&pixmap);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(col));
            painter.drawRect(pixmap.rect());

            icons << QIcon(pixmap);
            hues << std::max(-1, i);
        }
    }

    // create menu
    for (int i=0; i<icons.size(); ++i)
    {
        QAction * a = new QAction(icons[i], "", menu);
        a->setData(hues[i]);
        menu->addAction(a);
    }

    return menu;
}


} // namespace GUI
} // namespace MO
