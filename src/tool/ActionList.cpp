/** @file actionlist.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/9/2014</p>
*/

#include <QMenu>

#include "ActionList.h"

namespace MO {

ActionList::ActionList(const ActionList& other) :
    QList<QAction*>(other)
{
}

void ActionList::clearActions()
{
    for (auto i : *this)
        i->deleteLater();

    clear();
}


QAction * ActionList::addAction(const QString& name, QObject * parent)
{
    QAction * a = new QAction(name, parent);
    append(a);
    return a;
}

QAction * ActionList::addAction(const QIcon& icon, const QString& name, QObject * parent)
{
    QAction * a = new QAction(icon, name, parent);
    append(a);
    return a;
}

QAction * ActionList::addSeparator(QObject * parent)
{
    if (!isEmpty() && back()->isSeparator())
        return back();

    QAction * a = new QAction(parent);
    a->setSeparator(true);
    append(a);
    return a;
}

QAction * ActionList::addTitle(const QString& name, QObject * parent)
{
    QAction * a = new QAction(name, parent);
    QFont f(a->font());
    f.setBold(true);
    a->setFont(f);
    append(a);
    return a;
}

QAction * ActionList::addMenu(QMenu *menu, QObject * parent)
{
    QAction * a = new QAction(menu->title(), parent);
    a->setMenu(menu);
    append(a);
    return a;
}



} // namespace MO
