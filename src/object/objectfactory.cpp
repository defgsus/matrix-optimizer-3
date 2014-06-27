/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "objectfactory.h"
#include "object.h"

namespace MO {


ObjectFactory::ObjectFactory(QObject *parent) :
    QObject(parent)
{
}


Object * ObjectFactory::createObject(const QString &className)
{
    return new Object(className);
}

} // namespace MO
