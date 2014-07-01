/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QIcon>

#include "objectfactory.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "io/application.h"

#include "object.h"
#include "dummy.h"
#include "object/translation.h"
#include "object/axisrotation.h"
#include "scene.h"

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

const QIcon& ObjectFactory::iconForObject(const Object * o)
{
    static QIcon iconNone(":/icon/obj_none.png");
    static QIcon icon3d(":/icon/obj_3d.png");
    static QIcon iconParameter(":/icon/obj_parameter.png");
    static QIcon iconSoundSource(":/icon/obj_soundsource.png");
    static QIcon iconMicrophone(":/icon/obj_microphone.png");
    static QIcon iconCamera(":/icon/obj_camera.png");
    static QIcon iconTranslation(":/icon/obj_translation.png");
    static QIcon iconRotation(":/icon/obj_rotation.png");

    if (o->isTransformation())
    {
        if (qobject_cast<const Translation*>(o))
            return iconTranslation;
        if (qobject_cast<const AxisRotation*>(o))
            return iconRotation;
    }
    if (o->isCamera()) return iconCamera;
    if (o->isMicrophone()) return iconMicrophone;
    if (o->isSoundSource()) return iconSoundSource;
    if (o->isGl()) return icon3d;
    if (o->isParameter()) return iconParameter;

    return iconNone;

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

//    MO_DEBUG("registered object '" << obj->className() << "'");

    return true;
}

Object * ObjectFactory::createObject(const QString &className, bool createParameters)
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

    if (createParameters)
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
    Object * dummy = createObject(MO_OBJECTCLASSNAME_DUMMY);
    MO_ASSERT(dummy, "could not create Dummy object");
    return dummy;
}


QList<const Object*> ObjectFactory::possibleChildObjects(const Object * parent)
{
    QList<const Object*> list;

    for (auto &i : instance().objectMap_)
    {
        Object * o = i.second.get();

        if (!o->isValid() || o->isScene())
            continue;

        if (parent->canHaveChildren(o->type()))
            list.append(o);
    }

    return list;
}

bool ObjectFactory::canHaveChildObjects(const Object * parent)
{
    for (auto &i : instance().objectMap_)
    {
        Object * o = i.second.get();

        if (!o->isValid() || o->isScene())
            continue;

        if (parent->canHaveChildren(o->type()))
            return true;
    }

    return false;
}


} // namespace MO
