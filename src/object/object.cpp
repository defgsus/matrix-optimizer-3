/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>

#include "object.h"
#include "tool/stringmanip.h"
#include "io/error.h"

namespace MO {



Object::Object(const QString &className, QObject *parent) :
    QObject         (parent),
    className_      (className),
    idName_         (className),
    name_           (className),
    parentObject_   (0)
{
    MO_ASSERT(className.size(), "no class name given for Object");

    // tie in Object hierarchy
    if (auto o = dynamic_cast<Object*>(parent))
    {
        setParentObject(o);
    }
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

QString Object::getUniqueId(QString id)
{
    while (getChildObject(id))
    {
        increase_id_number(id, 1);
    }

    return id;
}

Object * Object::getChildObject(const QString &id, bool recursive)
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
