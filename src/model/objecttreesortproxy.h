/** @file objecttreesortproxy.h

    @brief custom filter and sorted for ObjectTreeModel

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2014</p>
*/

#ifndef MOSRC_MODEL_OBJECTTREESORTPROXY_H
#define MOSRC_MODEL_OBJECTTREESORTPROXY_H

#include <QSortFilterProxyModel>

namespace MO {



class ObjectTreeSortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ObjectTreeSortProxy(QObject *parent = 0);

signals:

public slots:

    void setObjectTypes(int flags);

protected:

    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    int objectTypes_;
};

} // namespace MO

#endif // MOSRC_MODEL_OBJECTTREESORTPROXY_H
