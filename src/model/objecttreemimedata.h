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
#include "object/util/audioobjectconnections.h"

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
    static const QString audioConMimeType;

    // ----------- info ----------------

    /** Returns true if at least one object is in the clipboard */
    static bool isObjectInClipboard();
    /** Returns the number of objects in the clipboard, or 0 */
    static int numObjectsInClipboard();
    /** Returns true if @p typeFlags matches the Object::Type of the
        first Object in clipboard */
    static bool isObjectTypeInClipboard(int typeFlags);

    //virtual QStringList formats() const;

    // ------------ store --------------

    /** Store a single tree/branch */
    void storeObjectTree(const Object * o);
    /** Store multiple trees/branches */
    void storeObjectTrees(const QList<const Object *> &o);
    /** Store multiple trees/branches */
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

    /** Restore the first tree/branch */
    Object * getObjectTree() const;
    /** Restore all trees/branches. */
    QList<Object*> getObjectTrees() const;
    /** Returns the top-level object IDs */
    QList<QString> getObjectTreeIds() const;

    /** Returns the audio connections previously stored */
    AudioObjectConnections * getAudioConnections(Object * rootObject = 0) const;
    bool hasAudioConnections() const;

    // ------ for drag/drop -------------

    // non-persistent data (only useable inside application)
    void setModelIndex(const QModelIndex & index) { index_ = index; }
    const QModelIndex& getModelIndex() const { return index_; }

protected:

    QModelIndex index_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
