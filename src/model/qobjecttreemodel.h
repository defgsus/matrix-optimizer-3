/** @file qobjecttreemodel.h

    @brief Model to display QObjects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef QOBJECTTREEMODEL_H
#define QOBJECTTREEMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

namespace MO {

class QObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QObjectTreeModel(QObject * rootObject = 0, QObject *parent = 0);

    void setRootObject(QObject * rootObject);
    QObject * rootObject() const { return rootObject_; }

    QObject * itemForIndex(const QModelIndex& index) const;

    // --- interface impl. ---

    //virtual QModelIndex createIndex(int row, int column, void *data) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:

public slots:

private:

    QModelIndex addObject_(QObject *);

    QObject * rootObject_;

    QStringList headerNames_;
};

} // namespace MO

#endif // QOBJECTTREEMODEL_H
