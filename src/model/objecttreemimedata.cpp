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

namespace {
static const QString objMimeType = "matrixoptimizer/object-tree";
}

ObjectTreeMimeData::ObjectTreeMimeData() :
    QMimeData()
{
    //setParent(parent);
}

const QString& ObjectTreeMimeData::mimeType()
{
    return objMimeType;
}

QStringList ObjectTreeMimeData::formats() const
{
    return QStringList() << objMimeType;
}


void ObjectTreeMimeData::setObjectTreeData_(const QByteArray & bytes)
{
    setData(objMimeType, bytes);
}

QByteArray ObjectTreeMimeData::getObjectTreeData_() const
{
    return (data(objMimeType));
}

void ObjectTreeMimeData::setObjectTree(const Object * obj)
{
    QByteArray a;
    IO::DataStream io(&a, QIODevice::WriteOnly);
    obj->serializeTree(io);
    setObjectTreeData_(a);
}

Object * ObjectTreeMimeData::getObjectTree() const
{
    QByteArray a = getObjectTreeData_();
    IO::DataStream io(a);
    return Object::deserializeTree(io);
}



} // namespace MO
