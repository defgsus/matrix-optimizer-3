/** @file actionlist.h

    @brief list of QAction for convenience

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/9/2014</p>
*/

#ifndef MOSRC_TOOL_ACTIONLIST_H
#define MOSRC_TOOL_ACTIONLIST_H

#include <QList>
#include <QAction>

namespace MO {


class ActionList : public QList<QAction*>
{
public:
    ActionList() { }
    explicit ActionList(const ActionList& other);

    void clearActions();

    QAction * addAction(const QString& name, QObject * parent);

    QAction * addAction(const QIcon& icon, const QString& name, QObject * parent);

    QAction * addSeparator(QObject * parent);

    QAction * addTitle(const QString& name, QObject * parent);

    QAction * addMenu(QMenu * menu, QObject * parent);

};


} // namespace MO


#endif // MOSRC_TOOL_ACTIONLIST_H
