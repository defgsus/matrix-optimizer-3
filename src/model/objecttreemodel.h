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

    Object * objectForIndex(const QModelIndex& index) const;
    QModelIndex indexForObject(Object*);

    // --- interface impl. ---

    QModelIndex index(
    int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QVariant headerData(
    int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant&,
                         int role = Qt::EditRole) override;

    // -- drag/drop impl --

    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                         int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;

    // Needed for drag-move
    bool removeRows(int row, int count, const QModelIndex &parent) override;

signals:

public slots:

private:

    struct Private;
    Private* p_;
};

} // namespace MO

#endif // OBJECTTREEMODEL_H
