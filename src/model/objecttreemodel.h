/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/13/2016</p>
*/

#ifndef OBJECTTREEMODEL_H
#define OBJECTTREEMODEL_H

#include <QAbstractItemModel>
#include "object/object_fwd.h"

namespace MO {

class ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ObjectTreeModel(Object * rootObject = 0, QObject *parent = 0);
    ~ObjectTreeModel();

    void setRootObject(Object * rootObject);
    Object * rootObject() const;

    Object * itemForIndex(const QModelIndex& index) const;

    // --- interface impl. ---

    virtual QModelIndex index(
            int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;

    virtual QVariant headerData(
            int section, Qt::Orientation orientation, int role) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:

public slots:

private:

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // OBJECTTREEMODEL_H
