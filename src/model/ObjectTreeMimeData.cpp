/** @file objecttreemimedata.cpp

    @brief QMimeData for serialized Object trees

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#include <QStringList>
#include <QClipboard>

#include "ObjectTreeMimeData.h"
#include "object/Object.h"
#include "object/Scene.h"
//#include "object/util/audioobjectconnections.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"
#include "io/Application.h"

namespace MO {

const QString ObjectTreeMimeData::objectMimeType = "matrixoptimizer/object-tree";
const QString ObjectTreeMimeData::typeMimeType = "matrixoptimizer/object-type";
const QString ObjectTreeMimeData::numMimeType = "matrixoptimizer/num-object-tree";
const QString ObjectTreeMimeData::orderMimeType = "matrixoptimizer/object-tree-order";
const QString ObjectTreeMimeData::audioConMimeType = "matrixoptimizer/audio-connections";


QList<Object*> ObjectTreeMimeData::filterTopLevel(const QList<Object*>& list)
{
    return filterTopLevel(list.toSet());
}

QList<Object*> ObjectTreeMimeData::filterTopLevel(const QSet<Object*>& list)
{
    QList<Object*> toplevel;
    for (Object * o : list)
    {
        Object * tl = o;
        while ((o = o->parentObject()))
        {
            if (list.contains(o))
            {
                tl = nullptr;
                break;
            }
        }

        if (tl)
            toplevel << tl;
    }
    return toplevel;
}


ObjectTreeMimeData::ObjectTreeMimeData() :
    QMimeData   ()
{

}
/*
QStringList ObjectTreeMimeData::formats() const
{
    return QStringList() << objectMimeType;
}
*/
bool ObjectTreeMimeData::isObjectInClipboard()
{
    return application()->clipboard()->mimeData()->formats()
            .contains(objectMimeType);
}

int ObjectTreeMimeData::numObjectsInClipboard()
{
    if (!isObjectInClipboard())
        return 0;

    return static_cast<const ObjectTreeMimeData*>(
                application()->clipboard()->mimeData())->getNumObjects();
}

bool ObjectTreeMimeData::isObjectTypeInClipboard(int typeFlags)
{
    if (!application()->clipboard()->mimeData()->formats()
                .contains(objectMimeType))
        return false;

    Object::Type type = static_cast<const ObjectTreeMimeData*>(
                application()->clipboard()->mimeData())->getObjectType();

    return type & typeFlags;
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

    // store audio connections
    a.clear();
    io.device()->reset();
    if (!o.isEmpty())
    {
        auto s = o[0]->sceneObject();
        auto con = s->audioConnections()->reducedTo(o);
        //con.dump(std::cout);
        con.serialize(io);
    }
    setData(audioConMimeType, a);
}

void ObjectTreeMimeData::storeObjectTrees(const QList<Object*>& objs)
{
    // convert to list of <const Object*>
    QList<const Object*> list;
    for (auto o : objs)
        list << o;
    storeObjectTrees(list);
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

    // return first object
    Object * obj = Object::deserializeTree(io);

    // attach audio connections
    if (hasAudioConnections())
    {
        if (auto con = getAudioConnections(obj))
            obj->assignAudioConnections(con);
    }

    return obj;
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

    // attach audio connections
    if (!list.isEmpty() && hasAudioConnections())
    {
        if (auto con = getAudioConnections())
        {
            //for (auto o : list)
            //    con->assignPointers(o);
            list.last()->assignAudioConnections(con);
        }
    }

    return list;
}

QList<QString> ObjectTreeMimeData::getObjectTreeIds() const
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

    QList<QString> list;
    for (int i=0; i<num; ++i)
    {
        auto obj = Object::deserializeTree(io);
        list.append( obj->idName() );
        obj->releaseRef("ObjectTreeMimeData getObjectTreeIds()");
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
    return !data(orderMimeType).isEmpty();
}

bool ObjectTreeMimeData::hasAudioConnections() const
{
    return !data(audioConMimeType).isEmpty();
}

AudioObjectConnections * ObjectTreeMimeData::getAudioConnections(Object * root) const
{
    QByteArray a = data(audioConMimeType);
    if (a.isEmpty())
        MO_IO_ERROR(READ, "No audio-connections data in clipboard");
    IO::DataStream io(a);

    auto con = new AudioObjectConnections;
    if (root)
        con->deserialize(io, root);
    else
        con->deserialize(io);
    return con;
}

} // namespace MO
