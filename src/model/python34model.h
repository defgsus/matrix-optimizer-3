/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/4/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34_

#ifndef MOSRC_MODEL_PYTHON34MODEL_H
#define MOSRC_MODEL_PYTHON34MODEL_H

#include <QObject>

#include <QAbstractItemModel>
#include <QStringList>

namespace MO {

class Python34Model : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit Python34Model(QObject *parent = 0);

    void setRootObject(void* rootObject);
    void* rootObject() const { return rootObject_; }

    void* nodeForIndex(const QModelIndex&) const;

    QList<QModelIndex> getAllIndices() const;

    // --- interface impl. ---

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;


signals:

public slots:

private:

    struct PyNode;

    void addIndices_(void* node, QList<QModelIndex>& list) const;
    /*void numChildren_(void* node) const;
    void* children_(void* node, size_t idx) const;
    void* parent_(void* node) const;
    void* indexOf_(void* parenNode, void* childNode) const;
    */

    PyNode rootObject_;
    QStringList headerNames_;
};

#endif // MOSRC_MODEL_PYTHON34MODEL_H

#endif // MO_ENABLE_PYTHON34
