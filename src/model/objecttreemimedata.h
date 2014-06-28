/** @file objecttreemimedata.h

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
#define MOSRC_OBJECT_OBJECTTREEMIMEDATA_H

#include <QMimeData>
#include <QModelIndex>

namespace MO {

class Object;

class ObjectTreeMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit ObjectTreeMimeData();

    static const QString& mimeType();

    virtual QStringList formats() const;

    void setObjectTree(const Object * o);
    Object * getObjectTree() const;

    void setModelIndex(const QModelIndex & index) { index_ = index; }
    const QModelIndex& getModelIndex() const { return index_; }

protected:

    void setObjectTreeData_(const QByteArray&);
    QByteArray getObjectTreeData_() const;

    QModelIndex index_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
