/** @file objectfactory.cpp

    @brief factory for MO::Object classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#include <QDebug>
#include <QIcon>
#include <QBitmap>
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QFileInfo>

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
#include "param/parameters.h"
#include "object/scene.h"
#include "object/control/trackfloat.h"
#include "object/control/sequencefloat.h"
#include "object/control/modulatorobjectfloat.h"
#include "object/synthesizer.h"
#include "object/textobject.h"
#include "object/visual/model3d.h"
#include "object/texture/imageto.h"
#include "geom/geometryfactorysettings.h"
#include "util/audioobjectconnections.h"
#include "audio/filterao.h"
#include "io/files.h"

namespace MO {

namespace {

    static QString MO_SCENEFILE_HEADER( "matrix-optimizer-scene" );
    static int     MO_SCENEFILE_VERSION( 1 );

    static QString MO_OBJECTFILE_HEADER( "matrix-optimizer-object" );
    static int     MO_OBJECTFILE_VERSION( 2 );

}

ObjectFactory * ObjectFactory::instance_ = 0;

ObjectFactory::ObjectFactory() :
    QObject(application())
{
}

ObjectFactory& ObjectFactory::instance()
{
    if (!instance_)
        instance_ = new ObjectFactory();

    return *instance_;
}

int ObjectFactory::objectPriority(const Object *o)
{
    // need to adjust ObjectFactory::objectPriorityName() as well

    if (o->isTransformation())
        return 5;
    if (o->isTexture())
        return 3;
    if (o->isGl() || o->isLightSource() || o->isGeometry())
        return 4;
    // 2 = meta
    if (o->type() & Object::TG_MODULATOR)
        return 1;
    if (o->isAudioUnit() || o->isAudioObject())
        return 0;
    return 2;
}

QString ObjectFactory::objectPriorityName(int priority)
{
    switch (priority)
    {
        case 5: return tr("Transformation");
        case 4: return tr("Visual");
        case 3: return tr("Texture");
        case 2: return tr("Meta");
        case 1: return tr("Control");
        case 0: return tr("Audio");
        default: return QString();
    }
}

int ObjectFactory::typeForClass(const QString &className)
{
    auto i = instance().objectMap_.find(className);
    return i == instance().objectMap_.end()
            ? -1
            : i->second->type();
}

const Object * ObjectFactory::getObject(const QString &className)
{
    auto i = instance().objectMap_.find(className);
    return i == instance().objectMap_.end()
            ? 0
            : i->second.get();
}

int ObjectFactory::hueForObject(int type)
{
    if (type == Object::T_DUMMY)
        return 20;
    else
    if (type & Object::TG_TRANSFORMATION)
        return 240;
    else
    if (type == Object::T_LIGHTSOURCE)
        return 60;
    else
    if (type == Object::T_GEOMETRY)
        return 200;
    else
    if (type == Object::T_SHADER || type == Object::T_TEXTURE)
        return 45;
    else
    if (type & Object::TG_TRACK)
        return 100;
    else
    if (type & Object::TG_SEQUENCE)
        return 140;
    else
    if (type & Object::TG_MODULATOR_OBJECT)
        return 170;
    else
    if (type & Object::T_AUDIO_OBJECT)
        return 350;
    else
    if (type & Object::T_SOUNDSOURCE)
        return 290;
    else
    if (type & Object::T_SOUND_OBJECT)
        return 290;
    else
        return -1;
}

QColor ObjectFactory::colorForObject(const Object * o, bool darkSet)
{
    const bool active = o->activeAtAll();

    int bright = darkSet? 48 : 200;

    int hue = (o->hasAttachedData(Object::DT_HUE))
            ? o->getAttachedData(Object::DT_HUE).toInt()
            : hueForObject(o->type());

    if (!active)
        bright += darkSet? 100 : -90;

    if (hue == -1)
        return QColor(bright, bright, bright);

    int sat = active ? 128 : 28;

    if (!o->isValid())
    {
        hue = 20;
        sat = 200;
    }

    return QColor::fromHsl(hue, sat, bright);
}
#if 0
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
    static QIcon iconClip(":/icon/obj_clip.png");
    static QIcon iconClipCont(":/icon/obj_clipcontroller.png");
    static QIcon iconAudio(":/icon/obj_audio.png");

/*    if (qobject_cast<const Synthesizer*>(o))
        return iconMusicNote;
*/
    if (o->isClip())
        return iconClip;

    if (o->isClipController())
        return iconClipCont;

    if (o->isAudioObject())
        return iconAudio;

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

    if (o->isAudioObject())
    {
        return iconParameter;
    }

    if (o->isAudioObject())
    {
//        if (qobject_cast<const EnvelopeUnit*>(o))
//            return iconAUEnv;
        if (qobject_cast<const FilterAO*>(o))
            return iconAUFilter;
        //if (qobject_cast<const FilterBankUnit*>(o))
//            return iconAUFilterBank;
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
    static QIcon iconClip(":/icon/obj_clip.png");
    static QIcon iconClipCont(":/icon/obj_clipcontroller.png");
    static QIcon iconAudio(":/icon/obj_audio.png");

    switch (type)
    {
        case Object::T_OBJECT: return icon3d;
        case Object::T_TRANSFORMATION: return iconTranslation;
        case Object::T_MICROPHONE: return iconMicrophone;
        case Object::T_CAMERA: return iconCamera;
        case Object::T_SOUNDSOURCE: return iconSoundSource;
        case Object::T_CLIP: return iconClip;
        case Object::T_CLIP_CONTROLLER: return iconClipCont;
        case Object::T_AUDIO_OBJECT: return iconAudio;
    }
    if (type & Object::TG_TRACK) return iconTrack;
    if (type & Object::TG_FLOAT) return iconParameter;

    return iconNone;
}

QIcon ObjectFactory::iconForObject(const Object * obj, QColor color, const QSize& size)
{
    QSize si = size;
    QIcon org = iconForObject(obj);
    if (si.isEmpty())
        si = QSize(128, 128);
    QPixmap pix(si);
    pix.fill(color);
    pix.setMask(org.pixmap(si).mask());
    return QIcon(pix);
}
#endif
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

    ObjectPrivate::setObjectId(obj, obj->className());
    if (obj->name().isEmpty())
        obj->setName(className);

    if (createParametersAndObjects)
    {
        obj->createParameters();

        //obj->createOutputs();
    }

    return obj;
}

Object * ObjectFactory::createObjectFromUrl(const QUrl& url)
{
    if (!url.isLocalFile())
        return 0;

    // get filename
    QString fn = url.toString(QUrl::PreferLocalFile | QUrl::NormalizePathSegments);
    if (fn.isEmpty())
        return 0;

#ifdef Q_OS_WIN
    // Well..., sometimes above yields something like /C:/bla/blub
    while (fn.startsWith("/"))
        fn.remove(0, 1);
#endif
    // filename only
    const QString shortfn = QFileInfo(fn).fileName();

    // get file type
    auto ft = IO::guessFiletype(fn);

    // images
    if (ft == IO::FT_TEXTURE)
    if (auto o = create_object<ImageTO>(shortfn))
    {
        o->setImageFilename(fn);
        return o;
    }

    // audio files
    if (ft == IO::FT_SOUND)
    if (auto o = create_object<SequenceFloat>(shortfn))
    {
        o->setSequenceType(SequenceFloat::ST_SOUNDFILE);
        o->setSoundFilename(fn);
        return o;
    }

    // text files
    if (ft == IO::FT_TEXT)
    if (auto o = create_object<TextObject>(shortfn))
    {
        QFile f(fn);
        if (!f.open(QFile::ReadOnly | QFile::Text))
            MO_IO_ERROR(READ, tr("Could not open text file '%1' for reading\n%2")
                        .arg(shortfn)
                        .arg(f.errorString()));
        /** @todo Support encoding options for reading general text files.
            Not necessarily here but in general. */
        o->setText(QString::fromUtf8(f.readAll()), TT_PLAIN_TEXT);
        return o;
    }

    // object templates
    if (ft == IO::FT_OBJECT_TEMPLATE)
    {
        auto o = loadObject(fn);
        return o;
    }

    // geometry-presets
    if (ft == IO::FT_GEOMETRY_SETTINGS)
    {
        GEOM::GeometryFactorySettings set(0);
        set.loadFile(fn);
        auto o = create_object<Model3d>(shortfn);
        o->setGeometrySettings(set);
        return o;
    }

    return 0;
}

Scene * ObjectFactory::createSceneObject()
{
    Scene * s = create_object<Scene>("Scene");
    MO_ASSERT(s, "could not create Scene object");
    return s;
}

TrackFloat * ObjectFactory::createTrackFloat(const QString &name)
{
    TrackFloat * t = qobject_cast<TrackFloat*>(createObject("TrackFloat"));
    MO_ASSERT(t, "could not create TrackFloat object");

    if (!name.isEmpty())
        t->setName(name);

    return t;
}

SequenceFloat * ObjectFactory::createSequenceFloat(const QString& name)
{
    SequenceFloat * seq = qobject_cast<SequenceFloat*>(createObject("SequenceFloat"));
    MO_ASSERT(seq, "could not create SequenceFloat object");
    if (!name.isEmpty())
        seq->setName(name);
    return seq;
}

ModulatorObjectFloat * ObjectFactory::createModulatorObjectFloat(const QString &name)
{
    ModulatorObjectFloat * o =
            qobject_cast<ModulatorObjectFloat*>(createObject("ModulatorObjectFloat"));
    MO_ASSERT(o, "could not create ModulatorObjectFloat object");

    if (!name.isEmpty())
        o->setName(name);

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

    std::sort(list.begin(), list.end(), [](const Object * l, const Object * r)
    {
        return l->name() < r->name();
    });
    return list;
}

/*
QString ObjectFactory::getParameterDoc(const QString &className)
{
    auto o = instance().createObject(className);
    if (!o)
        return QString();

    QString doc = o->params()->getParameterDoc();
    delete o;
    return doc;
}
*/


int ObjectFactory::getBestInsertIndex(Object *parent, Object *newChild, int idx)
{
    if (parent->childObjects().isEmpty())
        return 0;

    const int num = parent->childObjects().size();

    if (idx < 0 || idx >= num)
        idx = num;

    // find place according to priority
    const int p = ObjectFactory::objectPriority(newChild);
    for (int i = 0; i < num; ++i)
    {
        const int pi = ObjectFactory::objectPriority( parent->childObjects()[i] );
        if (idx <= i && pi <= p)
            return i;
        if (pi < p)
            return i;
    }
    return num;

#if (0)
    for (int i = num - 1; i > 0; --i)
    {
        //const int pi = Object::objectPriority( parent->childObjects()[i] );
        const int pim = Object::objectPriority( parent->childObjects()[i-1] );
        // matches index and priority?
        if (pim >= p && idx >= i)
            return i;
        /*const int pim = Object::objectPriority( parent->childObjects()[i-1] );
        if (pim >= p && idx < i)
            return i;*/
    }
    return 0;
#endif
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

    // v2 - store audio connections
    AudioObjectConnections acon;
    if (auto s = obj->sceneObject())
        acon = s->audioConnections()->reducedTo(obj);

    if (acon.isEmpty())
        io << (quint8)0;
    else
    {
        io << (quint8)1;
        acon.serialize(io);
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
    int ver;
    try
    {
        ver = io.readHeader(MO_OBJECTFILE_HEADER, MO_OBJECTFILE_VERSION);
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

    // audio connections
    if (ver >= 2)
    {
        quint8 audio_present;
        io >> audio_present;
        if (audio_present != 0)
        {
            auto acon = new AudioObjectConnections();
            acon->deserialize(io, o);
            // store in object so scene can incorporate them on add
            o->assignAudioConnections(acon);
        }
    }

    // removes modulators that are outside of our own tree
    // Important for loading templates which where previously connected
    // to stuff which is not available now or is something different
    o->removeOutsideModulators(true);

    //o->clearNullModulators(true);

    return o;
}


void ObjectFactory::storeObjectTemplate(Object * obj)
{
    QString fn = IO::Files::getSaveFileName(IO::FT_OBJECT_TEMPLATE, 0);
    if (fn.isEmpty())
        return;

    try
    {
        saveObject(fn, obj);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("io error"),
                              tr("Could not save the object template\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
}

Object * ObjectFactory::loadObjectTemplate()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_OBJECT_TEMPLATE, 0);
    if (fn.isEmpty())
        return 0;

    try
    {
        Object * o = ObjectFactory::loadObject(fn);
        return o;
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("io error"),
                              tr("Could not load the object template\n%1\n%2")
                              .arg(fn).arg(e.what()));
    }
    return 0;
}


} // namespace MO

