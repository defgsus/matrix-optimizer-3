/** @file objecttreemimedata.cpp

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/
//#include <QDebug>
#include <QStringList>

#include "objecttreemimedata.h"
#include "object/object.h"
#include "io/datastream.h"
#include "io/error.h"

namespace MO {

const QString ObjectTreeMimeData::objectMimeType = "matrixoptimizer/object-tree";
const QString ObjectTreeMimeData::typeMimeType = "matrixoptimizer/object-type";
const QString ObjectTreeMimeData::numMimeType = "matrixoptimizer/num-object-tree";
const QString ObjectTreeMimeData::orderMimeType = "matrixoptimizer/object-tree-order";


ObjectTreeMimeData::ObjectTreeMimeData() :
    QMimeData   ()
{
    //setParent(parent);
}

QStringList ObjectTreeMimeData::formats() const
{
    return QStringList() << objectMimeType;
}



void ObjectTreeMimeData::storeObjectTree(const Object * obj)
{
    storeObjectTrees(QList<const Object*>() << obj);
}

void ObjectTreeMimeData::storeObjectTrees(const QList<const Object *>& o)
{
    QByteArray a;
    IO::DataStream io(&a, QIODevice::WriteOnly);

    // store number
    io << (qint32)o.size();
    setData(numMimeType, a);

    // store types
    a.clear();
    io.device()->reset();
    io << (qint32)o.size();
    for (int i=0; i<o.size(); ++i)
    {
        io << (qint64)o[i]->type();
    }
    setData(typeMimeType, a);

    // store objects
    a.clear();
    io.device()->reset();
    io << (qint32)o.size();
    for (int i=0; i<o.size(); ++i)
    {
        o[i]->serializeTree(io);
    }
    setData(objectMimeType, a);
}

void ObjectTreeMimeData::storeObjectTrees(const QList<Object *>& o)
{
    QByteArray a;
    IO::DataStream io(&a, QIODevice::WriteOnly);

    // store number
    io << (qint32)o.size();
    setData(numMimeType, a);

    // store types
    a.clear();
    io.device()->reset();
    io << (qint32)o.size();
    for (int i=0; i<o.size(); ++i)
    {
        io << (qint64)o[i]->type();
    }
    setData(typeMimeType, a);

    // store objects
    a.clear();
    io.device()->reset();
    io << (qint32)o.size();
    for (int i=0; i<o.size(); ++i)
    {
        o[i]->serializeTree(io);
    }
    setData(objectMimeType, a);
}

void ObjectTreeMimeData::storeOrder(const QList<int>& order)
{
    QByteArray a;
    IO::DataStream io(&a, QIODevice::WriteOnly);

    io << order;
    setData(orderMimeType, a);
}


int ObjectTreeMimeData::getNumObjects() const
{
    QByteArray a = data(numMimeType);
    if (a.size() < (int)sizeof(qint32))
        return 0;
    IO::DataStream io(a);

    qint32 num;
    io >> num;
    return num;
}


Object::Type ObjectTreeMimeData::getObjectType() const
{
    QByteArray a = data(typeMimeType);
    if (a.size() < (int)(sizeof(qint32) + sizeof(qint64)))
        return Object::T_NONE;
    IO::DataStream io(a);

    qint32 num;
    io >> num;
    if (num<1)
        return Object::T_NONE;

    qint64 type;
    io >> type;
    return (Object::Type)(type);
}

QList<Object::Type> ObjectTreeMimeData::getObjectTypes() const
{
    QByteArray a = data(typeMimeType);
    if (a.size() < (int)(sizeof(qint32) + sizeof(qint64)))
        return QList<Object::Type>() << Object::T_NONE;
    IO::DataStream io(a);

    qint32 num;
    io >> num;
    if (num<1)
        return QList<Object::Type>() << Object::T_NONE;

    QList<Object::Type> list;
    qint64 type;
    for (int i=0; i<num; ++i)
    {
        io >> type;
        list.append((Object::Type)type);
    }
    return list;
}


Object * ObjectTreeMimeData::getObjectTree() const
{
    QByteArray a = data(objectMimeType);
    if (a.isEmpty())
        MO_IO_ERROR(READ, "No object data in clipboard");
    IO::DataStream io(a);

    // read number
    qint32 num;
    io >> num;
    if (num < 1)
        MO_IO_ERROR(READ, "No objects stored in clipboard");

    // return first
    return Object::deserializeTree(io);
}

QList<Object*> ObjectTreeMimeData::getObjectTrees() const
{
    QByteArray a = data(objectMimeType);
    if (a.isEmpty())
        MO_IO_ERROR(READ, "No object data in clipboard");
    IO::DataStream io(a);

    // read number
    qint32 num;
    io >> num;
    if (num < 1)
        MO_IO_ERROR(READ, "No objects stored in clipboard");

    QList<Object*> list;
    for (int i=0; i<num; ++i)
    {
        list.append( Object::deserializeTree(io) );
    }
    return list;
}

QList<int> ObjectTreeMimeData::getOrder() const
{
    QByteArray a = data(orderMimeType);
    IO::DataStream io(a);

    QList<int> order;
    io >> order;
    return order;
}

bool ObjectTreeMimeData::hasOrder() const
{
    return data(orderMimeType).size();
}


} // namespace MO
