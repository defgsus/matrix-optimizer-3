#ifndef MOSRC_MODEL_CSGMODEL_H
#define MOSRC_MODEL_CSGMODEL_H

#include <QAbstractItemModel>
#include <QStringList>
#include <QJsonObject>

namespace MO {

class CsgBase;
class CsgRoot;

class CsgTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CsgTreeModel(QObject *parent = 0);

    void setRootObject(CsgBase * rootObject);
    CsgBase* rootObject() const { return rootObject_; }

    CsgBase* nodeForIndex(const QModelIndex&) const;

    // --- interface impl. ---

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

signals:

public slots:

private:

    CsgBase * rootObject_;
    QStringList headerNames_;
};

} // namespace MO


#endif // MOSRC_MODEL_CSGMODEL_H
