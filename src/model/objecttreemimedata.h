/** @file objecttreemimedata.h

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
#define MOSRC_OBJECT_OBJECTTREEMIMEDATA_H

#include <QMimeData>

namespace MO {

class Object;

class ObjectTreeMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit ObjectTreeMimeData();

    virtual QStringList formats() const;

    void setObjectTree(const Object * o);
    Object * getObjectTree() const;

    void setObjectTreeData(const QByteArray&);
    QByteArray getObjectTreeData() const;

};

} // namespace MO

#endif // MOSRC_OBJECT_OBJECTTREEMIMEDATA_H
