/** @file objecttreemimedata.cpp

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QStringList>

#include "objecttreemimedata.h"
#include "object/object.h"
#include "io/datastream.h"

namespace MO {

const QString ObjectTreeMimeData::objectMimeType = "matrixoptimizer/object-tree";
const QString ObjectTreeMimeData::typeMimeType = "matrixoptimizer/object-type";


ObjectTreeMimeData::ObjectTreeMimeData() :
    QMimeData   ()
{
    //setParent(parent);
}

QStringList ObjectTreeMimeData::formats() const
{
    return QStringList() << objectMimeType << typeMimeType;
}


void ObjectTreeMimeData::setObjectTreeData_(const QByteArray & bytes)
{
    setData(objectMimeType, bytes);
}

QByteArray ObjectTreeMimeData::getObjectTreeData_() const
{
    return (data(objectMimeType));
}

Object::Type ObjectTreeMimeData::getObjectType() const
{
    QByteArray a = data(typeMimeType);
    if (a.isEmpty())
        return (Object::Type)(-1);
    return (Object::Type)(a.toInt());
}

void ObjectTreeMimeData::setObjectTree(const Object * obj)
{
    QByteArray a;
    IO::DataStream io(&a, QIODevice::WriteOnly);
    obj->serializeTree(io);
    setObjectTreeData_(a);

    // store type
    a.clear();
    a.append(QString::number(obj->type()));
    setData(typeMimeType, a);
}

Object * ObjectTreeMimeData::getObjectTree() const
{
    QByteArray a = getObjectTreeData_();
    IO::DataStream io(a);
    return Object::deserializeTree(io);
}



} // namespace MO
