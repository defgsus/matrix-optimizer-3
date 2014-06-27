/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "objectfactory.h"
#include "object.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/application.h"

namespace MO {

ObjectFactory * ObjectFactory::instance_ = 0;

ObjectFactory::ObjectFactory() :
    QObject(application)
{
}

ObjectFactory& ObjectFactory::instance()
{
    if (!instance_)
        instance_ = new ObjectFactory();
    return *instance_;
}

Object * ObjectFactory::createObject(const QString &className)
{
    return new Object(className);
}


} // namespace MO
