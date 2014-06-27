/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#include "objectfactory.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/application.h"

#include "object.h"
#include "dummy.h"
#include "parameter.h"
#include "soundsource.h"
#include "microphone.h"
#include "camera.h"

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
    Object * obj;

    if (className == "Camera")
        obj = new Camera();
    else if (className == "Microphone")
        obj = new Microphone();
    else if (className == "SoundSource")
        obj = new SoundSource();
    else
        obj = new Parameter();

    // prepare object
    obj->idName_ = obj->className();
    obj->name_ = className;

    return obj;
}

Object * ObjectFactory::createDummy()
{
    return new Dummy();
}

} // namespace MO
