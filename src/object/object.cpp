/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>

#include "object.h"
#include "objectfactory.h"
#include "tool/stringmanip.h"
#include "io/error.h"
#include "io/datastream.h"
#include "io/log.h"


namespace MO {



Object::Object(QObject *parent) :
    QObject         (parent),
    parentObject_   (0)
{
    // tie into Object hierarchy
    if (auto o = dynamic_cast<Object*>(parent))
    {
        setParentObject(o);
    }
}

// --------------------- io ------------------------

void Object::serializeTree(IO::DataStream & io)
{
    MO_DEBUG_IO("Object('"<<idName()<<"')::serializeTree()");

    // default header
    io.writeHeader("object-tree", 1);

    // default object info
    io << className() << idName() << name();

    // keep position to go back later
    const qint64 startPos = io.device()->pos();
    // write temp skip marker
    io << (qint64)0;

    // write actual derived object
    serialize(io);

    // once in a while check stream for errors
    if (io.status() != QDataStream::Ok)
        MO_IO_ERROR(WRITE, "error serializing object '"<<idName()<<"'.\n"
                    "QIODevice error: '"<<io.device()->errorString()<<"'");

    const qint64 endPos = io.device()->pos();
    // write length of object
    io.device()->seek(startPos);
    io << (qint64)(endPos - startPos - sizeof(qint64));
    io.device()->seek(endPos);

    // write childs
    io << (qint32)childObjects_.size();
    for (auto o : childObjects_)
        o->serializeTree(io);
}

Object * Object::deserializeTree(IO::DataStream & io)
{
    MO_DEBUG_IO("Object::deserializeTree()");

    // read default header
    io.readHeader("object-tree", 1);

    // read default object info
    QString className, idName, name;
    io >> className >> idName >> name;

    qint64 objLength;
    io >> objLength;

    // create this object
    MO_DEBUG_IO("creating object '" << className << "'");
    Object * o = ObjectFactory::instance().createObject(className);

    if (o)
    {
        // read derived object data
        o->deserialize(io);

        // once in a while check stream for errors
        if (io.status() != QDataStream::Ok)
            MO_IO_ERROR(WRITE, "error deserializing object '"<<idName<<"'.\n"
                        "QIODevice error: '"<<io.device()->errorString()<<"'");
    }
    // skip object if class not found
    else
    {
        MO_IO_WARNING(VERSION_MISMATCH, "unknown object class '" << className << "' in stream");
        io.device()->seek(io.device()->pos() + objLength);

        o = ObjectFactory::createDummy();
    }

    // set default object info
    o->idName_ = idName;
    o->name_ = name;

    // iterate over childs
    quint32 numChilds;
    io >> numChilds;

    for (quint32 i=0; i<numChilds; ++i)
    {
        Object * child = deserializeTree(io);
        if (child)
            o->addObject(child);
    }

    return o;
}



// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    name_ = n;
}



// ------------- tree stuff ------------------------

const Object * Object::rootObject() const
{
    return parentObject_ ? parentObject_->rootObject() : this;
}

Object * Object::rootObject()
{
    return parentObject_ ? parentObject_->rootObject() : this;
}

void Object::setParentObject(Object *parent, int index)
{
    MO_ASSERT(parent, "no parent given for Object");
    MO_ASSERT(parent != parentObject_, "trying to add same object to parent");

    if (parent == parentObject_)
        return;

    // install in QObject tree (handle memory)
    setParent(parent);

    // remove from previous parent
    if (parentObject_)
    {
        parentObject_->takeChild_(this);
    }

    // assign
    parentObject_ = parent;

    // adjust idname
    idName_ = parentObject_->getUniqueId(idName_);

    // and add to child list
    parentObject_->addChildObject_(this, index);
}

Object * Object::addObject(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");
    MO_ASSERT(!childObjects_.contains(o), "duplicate addChild for '" << o->idName());

    o->setParentObject(this, index);

    return o;
}

Object * Object::addChildObject_(Object * o, int index)
{
    MO_ASSERT(o, "trying to add a NULL child");

    if (index < 0)
        childObjects_.append(o);
    else
        childObjects_.insert(index, o);

    return o;
}

void Object::takeChild_(Object *child)
{
    childObjects_.removeOne(child);
    /*
    for (auto i=childObjects_.begin(); i!=childObjects_.end(); ++i)
    if (*i == child)
    {
        childObjects_.erase(i);
        return;
    }*/
}

QString Object::getUniqueId(QString id) const
{
    MO_ASSERT(!id.isEmpty(), "unset object id detected, class='"
              << className() << "', name='" << name() << "'");

    if (id.isEmpty())
        id = "empty";

    while (getChildObject(id))
    {
        increase_id_number(id, 1);
    }

    return id;
}

Object * Object::getChildObject(const QString &id, bool recursive) const
{
    for (auto i : childObjects_)
        if (i->idName() == id)
            return i;

    if (recursive)
        for (auto i : childObjects_)
            if (Object * o = i->getChildObject(id, recursive))
                return o;

    return 0;
}

} // namespace MO
