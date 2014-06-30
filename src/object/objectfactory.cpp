/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include "objectfactory.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"

#include "object.h"
#include "dummy.h"
#include "parameter.h"
#include "soundsource.h"
#include "microphone.h"
#include "camera.h"
#include "scene.h"
#include "model3d.h"

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

bool ObjectFactory::registerObject(Object * obj)
{
    if (instance().objectMap_.find(obj->className())
            != instance().objectMap_.end())
    {
        MO_ASSERT(false, "duplicate object class name registered '" << obj->className() << "'");
        return false;
    }

    instance().objectMap_.insert(
        std::make_pair(obj->className(), std::shared_ptr<Object>(obj))
        );

    MO_DEBUG("registered object '" << obj->className() << "'");

    return true;
}

Object * ObjectFactory::createObject(const QString &className)
{
    auto it = instance().objectMap_.find(className);
    if (it == instance().objectMap_.end())
    {
        MO_ASSERT(false, "request for unknown object class '" << className <<"'");
        return 0;
    }

    Object * obj = it->second->cloneClass();

    // prepare object
    obj->idName_ = obj->className();
    if (obj->name_.isEmpty())
        obj->name_ = className;

    obj->createParameters();

    return obj;
}

Scene * ObjectFactory::createSceneObject()
{
    Scene * s = qobject_cast<Scene*>(createObject(MO_OBJECTCLASSNAME_SCENE));
    MO_ASSERT(s, "could not create Scene object");
    return s;
}

Object * ObjectFactory::createDummy()
{
    return new Dummy();
}

} // namespace MO
