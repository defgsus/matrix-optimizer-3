/** @file objecttreemodel.h

    @brief Tree Model for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>

    <p>debug-switches:<br/>
    MO_DISABLE_OBJECT_TREE_DRAG : to disable object drag/drop (for optirun bug)
    </p>
*/

#ifndef MOSRC_MODEL_OBJECTTREEMODEL_H
#define MOSRC_MODEL_OBJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QIcon>
#include <QFont>

#include "types/float.h"
#include "object/object_fwd.h"

namespace MO {

enum
{
    ObjectRole = Qt::UserRole + 1
};

class ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ObjectTreeModel(Object * rootObject = 0, QObject *parent = 0);

    void setRootObject(Object * rootObject);
    Object * rootObject() const { return rootObject_; }
    QModelIndex rootIndex() const;
    Object * objectForIndex(const QModelIndex& index) const;
//    void indexForObject(const Object * obj, QModelIndex &parent, int &row) const;
    QModelIndex indexForObject(const Object * obj) const;

    // --- interface impl. ---

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    // -- editing --
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

    // -- drag/drop --
    virtual QStringList mimeTypes() const;
    virtual QMimeData * mimeData(const QModelIndexList &indexes) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::DropActions supportedDragActions() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent);

signals:

public slots:

    // ---- act upon changes ---


    // ---- custom editing ----

    bool deleteObject(const QModelIndex&);

    QModelIndex addObject(const QModelIndex& parent, int indexInChild, Object * obj);

    SequenceFloat * createFloatSequence(TrackFloat * track, Double time);

private:

    Object * rootObject_;

    QStringList headerNames_;

    QFont boldFont_;
    QColor
        colorDefault_,
        colorTransformation_,
        colorInvalid_;
};

} // namespace MO

#endif // MOSRC_MODEL_OBJECTTREEMODEL_H
