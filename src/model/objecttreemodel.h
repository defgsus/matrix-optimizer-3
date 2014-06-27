/** @file objecttreemodel.h

    @brief Tree Model for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_MODEL_OBJECTTREEMODEL_H
#define MOSRC_MODEL_OBJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

namespace MO {

class Object;

class ObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ObjectTreeModel(Object * rootObject = 0, QObject *parent = 0);

    void setRootObject(Object * rootObject);
    Object * rootObject() const { return rootObject_; }

    Object * itemForIndex(const QModelIndex& index) const;

    // --- interface impl. ---

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

signals:

public slots:

private:

    Object * rootObject_;

    QStringList headerNames_;
};

} // namespace MO

#endif // MOSRC_MODEL_OBJECTTREEMODEL_H
