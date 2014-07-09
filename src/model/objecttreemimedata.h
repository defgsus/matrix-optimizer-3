/** @file objecttreemimedata.h

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
#define MOSRC_OBJECT_OBJECTTREEMIMEDATA_H

#include <QMimeData>
#include <QModelIndex>

#include "object/object.h"

namespace MO {

class ObjectTreeMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit ObjectTreeMimeData();

    static const QString objectMimeType;
    static const QString typeMimeType;
    static const QString numMimeType;
    static const QString orderMimeType;

    virtual QStringList formats() const;

    // ------------ store --------------

    /** Store a single tree */
    void storeObjectTree(const Object * o);
    /** Store multiple trees */
    void storeObjectTrees(const QList<const Object *> &o);
    /** Store multiple trees */
    void storeObjectTrees(const QList<Object *> &o);
    /** Store an int list */
    void storeOrder(const QList<int>& order);

    // ----------- restore -------------

    /** Returns the number of objects in clipboard */
    int getNumObjects() const;
    /** Returns the type of the first object */
    Object::Type getObjectType() const;
    /** Returns the types of objects */
    QList<Object::Type> getObjectTypes() const;
    /** Returns an int list */
    QList<int> getOrder() const;
    bool hasOrder() const;

    /** Restore the first tree */
    Object * getObjectTree() const;
    /** Restore all trees */
    QList<Object*> getObjectTrees() const;

    // ------ for drag/drop -------------

    // non-persistent data (only useable inside application)
    void setModelIndex(const QModelIndex & index) { index_ = index; }
    const QModelIndex& getModelIndex() const { return index_; }

protected:

    QModelIndex index_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
