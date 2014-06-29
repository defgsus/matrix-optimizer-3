/** @file qobjecttreemodel.h

    @brief Model to display QObjects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_MODEL_QOBJECTTREEMODEL_H
#define MOSRC_MODEL_QOBJECTTREEMODEL_H

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

    static QString objectTitle(QObject *);

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

    QObject * rootObject_;

    QStringList headerNames_;
};

} // namespace MO

#endif // MOSRC_MODEL_QOBJECTTREEMODEL_H
