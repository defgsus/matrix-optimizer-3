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
#include "transform/translation.h"
#include "transform/axisrotation.h"
#include "transform/scale.h"
#include "transform/shear.h"
#include "transform/look.h"
#include "transform/lookat.h"
#include "transform/mix.h"
#include "audio/envelopeunit.h"
#include "audio/filterunit.h"
#include "audio/filterbankunit.h"
#include "scene.h"
#include "trackfloat.h"
#include "sequencefloat.h"
#include "modulatorobjectfloat.h"
#include "synthesizer.h"

namespace MO {

namespace {

    static QString MO_SCENEFILE_HEADER( "matrix-optimizer-scene" );
    static int     MO_SCENEFILE_VERSION( 1 );

    static QString MO_OBJECTFILE_HEADER( "matrix-optimizer-object" );
    static int     MO_OBJECTFILE_VERSION( 1 );

}

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
    static QIcon iconTrack(":/icon/obj_track.png");
    static QIcon iconTranslation(":/icon/obj_translation.png");
    static QIcon iconRotation(":/icon/obj_rotation.png");
    static QIcon iconScale(":/icon/obj_scale.png");
    static QIcon iconShear(":/icon/obj_shear.png");
    static QIcon iconLook(":/icon/obj_look.png");
    static QIcon iconLookAt(":/icon/obj_lookat.png");
    static QIcon iconMix(":/icon/obj_mix.png");
    static QIcon iconGroup(":/icon/obj_group.png");
    static QIcon iconLight(":/icon/obj_light.png");
    static QIcon iconModulator(":/icon/obj_modulator.png");
    static QIcon iconAUEnv(":/icon/obj_au_env.png");
    static QIcon iconAUFilter(":/icon/obj_au_filter.png");
    static QIcon iconAUFilterBank(":/icon/obj_au_filterbank.png");
    static QIcon iconMusicNote(":/icon/music_note.png");


    if (qobject_cast<const Synthesizer*>(o))
        return iconMusicNote;

    if (o->isTransformation())
    {
        if (qobject_cast<const Translation*>(o))
            return iconTranslation;
        if (qobject_cast<const AxisRotation*>(o))
            return iconRotation;
        if (qobject_cast<const Scale*>(o))
            return iconScale;
        if (qobject_cast<const Shear*>(o))
            return iconShear;
        if (qobject_cast<const Look*>(o))
            return iconLook;
        if (qobject_cast<const LookAt*>(o))
            return iconLookAt;
        if (qobject_cast<const Mix*>(o))
            return iconMix;
    }

    if (o->isAudioUnit())
    {
        if (qobject_cast<const EnvelopeUnit*>(o))
            return iconAUEnv;
        if (qobject_cast<const FilterUnit*>(o))
            return iconAUFilter;
        if (qobject_cast<const FilterBankUnit*>(o))
            return iconAUFilterBank;
    }


    if (o->type() & Object::T_GROUP) return iconGroup;
    if (o->isTrack()) return iconTrack;
    if (o->type() & Object::TG_FLOAT) return iconParameter;
    if (o->isCamera()) return iconCamera;
    if (o->isMicrophone()) return iconMicrophone;
    if (o->isSoundSource()) return iconSoundSource;
    if (o->isGl()) return icon3d;
    if (o->isParameter()) return iconParameter;
    if (o->isLightSource()) return iconLight;
    if (o->isModulatorObject()) return iconModulator;

    return iconNone;
}

const QIcon& ObjectFactory::iconForObject(int type)
{
    static QIcon iconNone(":/icon/obj_none.png");
    static QIcon icon3d(":/icon/obj_3d.png");
    static QIcon iconParameter(":/icon/obj_parameter.png");
    static QIcon iconSoundSource(":/icon/obj_soundsource.png");
    static QIcon iconMicrophone(":/icon/obj_microphone.png");
    static QIcon iconCamera(":/icon/obj_camera.png");
    static QIcon iconTranslation(":/icon/obj_translation.png");
    static QIcon iconRotation(":/icon/obj_rotation.png");
    static QIcon iconScale(":/icon/obj_scale.png");
    static QIcon iconTrack(":/icon/obj_track.png");

    switch (type)
    {
        case Object::T_OBJECT: return icon3d;
        case Object::T_TRANSFORMATION: return iconTranslation;
        case Object::T_MICROPHONE: return iconMicrophone;
        case Object::T_CAMERA: return iconCamera;
        case Object::T_SOUNDSOURCE: return iconSoundSource;
    }
    if (type & Object::TG_TRACK) return iconTrack;
    if (type & Object::TG_FLOAT) return iconParameter;

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

Object * ObjectFactory::createObject(const QString &className, bool createParametersAndObjects)
{
    auto it = instance().objectMap_.find(className);
    if (it == instance().objectMap_.end())
    {
        MO_WARNING("request for unknown object class '" << className <<"'");
        return 0;
    }

    // create the class
    Object * obj = it->second->cloneClass();

    // --- prepare object ---

    obj->idName_ = obj->className();
    if (obj->name_.isEmpty())
        obj->name_ = className;

    if (createParametersAndObjects)
    {
        obj->createParameters();

        obj->createAudioSources();
        obj->createMicrophones();
        //obj->createOutputs();
    }

    return obj;
}

Scene * ObjectFactory::createSceneObject()
{
    Scene * s = qobject_cast<Scene*>(createObject("Scene"));
    MO_ASSERT(s, "could not create Scene object");
    return s;
}

TrackFloat * ObjectFactory::createTrackFloat(const QString &name)
{
    TrackFloat * t = qobject_cast<TrackFloat*>(createObject("TrackFloat"));
    MO_ASSERT(t, "could not create TrackFloat object");

    if (!name.isEmpty())
        t->name_ = t->idName_ = name;

    return t;
}

SequenceFloat * ObjectFactory::createSequenceFloat(const QString& name)
{
    SequenceFloat * seq = qobject_cast<SequenceFloat*>(createObject("SequenceFloat"));
    MO_ASSERT(seq, "could not create SequenceFloat object");
    if (!name.isEmpty())
        seq->name_ = seq->idName_ = name;
    return seq;
}

ModulatorObjectFloat * ObjectFactory::createModulatorObjectFloat(const QString &name)
{
    ModulatorObjectFloat * o =
            qobject_cast<ModulatorObjectFloat*>(createObject("ModulatorObjectFloat"));
    MO_ASSERT(o, "could not create ModulatorObjectFloat object");

    if (!name.isEmpty())
        o->name_ = o->idName_ = name;

    return o;
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

QList<const Object*> ObjectFactory::objects(int types)
{
    QList<const Object*> list;

    for (auto &i : instance().objectMap_)
    {
        Object * o = i.second.get();

        if (o->type() & types)
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

    saveScene(io, scene);
}

void ObjectFactory::saveScene(IO::DataStream & io, const Scene * scene)
{
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

    return loadScene(io);
}

Scene * ObjectFactory::loadScene(IO::DataStream &io)
{
    try
    {
        io.readHeader(MO_SCENEFILE_HEADER, MO_SCENEFILE_VERSION);
    }
    catch (const IoException &e)
    {
        MO_IO_WARNING(VERSION_MISMATCH,
                      "error reading scene\n"
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
        MO_IO_WARNING(VERSION_MISMATCH, "Expected scene, got "
                      << (o? o->className() : "NULL"));
        if (o)
            delete o;
        return 0;
    }
}





void ObjectFactory::saveObject(const QString &fn, const Object * obj)
{
    QFile file(fn);

    if (!file.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "could not create object file '" << fn << "'\n"
                    << file.errorString());

    IO::DataStream io(&file);

    saveObject(io, obj);
}

void ObjectFactory::saveObject(IO::DataStream & io, const Object * obj)
{
    io.writeHeader(MO_OBJECTFILE_HEADER, MO_OBJECTFILE_VERSION);

    io << (quint8)useCompression_;

    if (!useCompression_)
        obj->serializeTree(io);
    else
    {
        QByteArray data = obj->serializeTreeCompressed();
        io << data;
    }
}

Object * ObjectFactory::loadObject(const QString &fn)
{
    QFile file(fn);

    if (!file.open(QFile::ReadOnly))
        MO_IO_ERROR(READ, "could not open object file '" << fn << "'\n"
                    << file.errorString());

    IO::DataStream io(&file);

    return loadObject(io);
}

Object * ObjectFactory::loadObject(IO::DataStream &io)
{
    try
    {
        io.readHeader(MO_OBJECTFILE_HEADER, MO_OBJECTFILE_VERSION);
    }
    catch (const IoException &e)
    {
        MO_IO_WARNING(VERSION_MISMATCH,
                      "error reading object\n"
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

    return o;
}




} // namespace MO

