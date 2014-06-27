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

namespace MO {



Object::Object(const QString &idName, QObject *parent) :
    QObject         (parent),
    idName_         (idName),
    name_           (idName),
    parentObject_   (0)
{
    MO_ASSERT(idName.size(), "no idname given for Object");

    // tie in Object hierarchy
    if (auto o = dynamic_cast<Object*>(parent))
    {
        setParentObject(o);
    }
}

// --------------------- io ------------------------

void Object::serializeTree(IO::DataStream & io)
{
    // default header
    io.writeHeader("object-tree", 1);

    // default object info
    io << className() << idName() << name();

    // write actual derived object
    serialize(io);

    // write childs
    io << (qint32)childObjects_.size();
    for (auto o : childObjects_)
        o->serializeTree(io);
}

Object * Object::deserializeTree(IO::DataStream & io)
{
    io.readHeader("object-tree", 1);

    QString className, idName, name;
    io >> className >> idName >> name;

    Object * o = ObjectFactory::instance().createObject(className);

    /** @todo How to skip unknown objects in stream??? */
    if (!o) MO_IO_ERROR(READ, "unknown object class '" << className << "'in stream");

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
