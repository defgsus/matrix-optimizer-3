/** @file object.cpp

    @brief Base class of all MO Objects

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "object.h"
#include "tool/stringmanip.h"

namespace MO {



Object::Object(const QString &className, QObject *parent) :
    QObject     (parent),
    className_  (className),
    idName_     (className),
    name_       (className)
{
}



// ------------------ setter -----------------------

void Object::setName(const QString & n)
{
    name_ = n;
}



// ------------- children --------------------------

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
    return 0;
}

} // namespace MO
