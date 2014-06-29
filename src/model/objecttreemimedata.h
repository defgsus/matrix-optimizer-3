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

    static const QString& mimeType();

    virtual QStringList formats() const;

    void setObjectTree(const Object * o);
    Object * getObjectTree() const;

    Object::Type getObjectType() const { return type_; }

    void setModelIndex(const QModelIndex & index) { index_ = index; }
    const QModelIndex& getModelIndex() const { return index_; }

protected:

    void setObjectTreeData_(const QByteArray&);
    QByteArray getObjectTreeData_() const;

    QModelIndex index_;
    Object::Type type_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
