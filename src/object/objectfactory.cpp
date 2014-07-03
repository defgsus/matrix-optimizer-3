/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>
#include <QIcon>
#include <QFile>

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

static QString MO_SCENEFILE_HEADER( "matrix-optimizer-scene" );
static int     MO_SCENEFILE_VERSION( 1 );

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
    Scene * s = qobject_cast<Scene*>(createObject("Scene"));
    MO_ASSERT(s, "could not create Scene object");
    /*Object * sg = createObject(MO_OBJECTCLASSNAME_SEQUENCES);
    MO_ASSERT(sg, "could not create SequenceGroup object");
    s->addObject(sg);
    */
    return s;
}

Object * ObjectFactory::createDummy()
{
    Object * dummy = createObject("Dummy");
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


void ObjectFactory::saveScene(const QString &fn, const Scene * scene)
{
    QFile file(fn);

    if (!file.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "could not create scene file '" << fn << "'\n"
                    << file.errorString());

    IO::DataStream io(&file);

    io.writeHeader(MO_SCENEFILE_HEADER, MO_SCENEFILE_VERSION);

    io << (quint8)useCompression_;

    if (!useCompression_)
        scene->serializeTree(io);
    else
    {
        QByteArray data = scene->serializeTreeCompressed();
        io << data;
    }
}

Scene * ObjectFactory::loadScene(const QString &fn)
{
    QFile file(fn);

    if (!file.open(QFile::ReadOnly))
        MO_IO_ERROR(READ, "could not open scene file '" << fn << "'\n"
                    << file.errorString());

    IO::DataStream io(&file);

    try
    {
        io.readHeader(MO_SCENEFILE_HEADER, MO_SCENEFILE_VERSION);
    }
    catch (IoException &e)
    {
        MO_IO_WARNING(VERSION_MISMATCH,
                      "error reading scene file '" << fn << "'\n"
                      << e.what());
        return 0;
    }

    quint8 compressed;
    io >> compressed;

    Object * o;

    if (!compressed)
        o = Object::deserializeTree(io);
    else
    {
        QByteArray data;
        io >> data;
        o = Object::deserializeTreeCompressed(data);
    }

    if (Scene * scene = qobject_cast<Scene*>(o))
    {
        return scene;
    }
    else
    {
        MO_IO_WARNING(VERSION_MISMATCH, "no Scene in file '" << fn << "'");
        if (o)
            delete o;
        return 0;
    }

}

} // namespace MO
