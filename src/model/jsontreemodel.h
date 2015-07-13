/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#ifndef MOSRC_MODEL_JSONTREEMODEL_H
#define MOSRC_MODEL_JSONTREEMODEL_H


#include <QAbstractItemModel>
#include <QStringList>
#include <QJsonObject>

namespace MO {

class JsonTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit JsonTreeModel(QObject *parent = 0);

    void setRootObject(const QJsonObject& rootObject);
    const QJsonObject& rootObject() const { return rootObject_; }

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
    struct Wrapper;

    Wrapper * wrapperForIndex_(const QModelIndex& index) const;

    Wrapper * p_;

    QJsonObject rootObject_;

    QStringList headerNames_;
};

} // namespace MO

#endif // MOSRC_MODEL_JSONTREEMODEL_H
