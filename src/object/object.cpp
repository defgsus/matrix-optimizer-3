/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

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

void Object::setParentObject(Object *parent)
{
    MO_ASSERT(parent, "no parent given for Object");

    // install in QObject tree (handle memory)
    setParent(parent);

    // remove from previous parent
    if (parentObject_)
    {
        parentObject_->takeChild_(this);
    }

    parentObject_ = parent;
    // adjust idname
    idName_ = parentObject_->getUniqueId(idName_);
}

void Object::takeChild_(Object *child)
{
    for (auto i=childObjects_.begin(); i!=childObjects_.end(); ++i)
    if (*i == child)
    {
        childObjects_.erase(i);
        return;
    }
}

QString Object::getUniqueId(QString id)
{
    while (getChild(id))
    {
        increase_id_number(id, 1);
    }

    return id;
}

Object * Object::getChild(const QString &id, bool recursive)
{
    for (auto i : childObjects_)
        if (i->idName() == id)
            return i;

    if (recursive)
        for (auto i : childObjects_)
            if (Object * o = i->getChild(id, recursive))
                return o;

    return 0;
}


} // namespace MO
