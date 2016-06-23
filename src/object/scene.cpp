/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>
#include <QReadWriteLock>

#include "scene.h"
#include "util/scenelock_p.h"
#include "io/error.h"
#include "io/log_gl.h"
#include "io/log_tree.h"
#include "io/log_param.h"
#include "io/log_audio.h"
#include "io/datastream.h"
#include "object/util/objectfactory.h"
#include "object/param/modulator.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parametertimeline1d.h"
#include "object/visual/camera.h"
#include "object/visual/shaderobject.h"
#include "object/texture/textureobjectbase.h"
#include "object/control/track.h"
#include "object/control/sequencefloat.h"
#include "object/control/clipcontroller.h"
#include "object/microphone.h"
#include "object/visual/lightsource.h"
#include "object/ascriptobject.h"
#include "object/control/modulatorobjectfloat.h"
#include "object/util/objectdsppath.h"
#include "object/util/objecteditor.h"
#include "object/util/audioobjectconnections.h"
#include "object/util/scenesignals.h"
#include "audio/audiodevice.h"
#include "audio/audiosource.h"
#include "gl/context.h"
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "gl/screenquad.h"
#include "gl/texture.h"
#include "gl/rendersettings.h"
#include "gl/scenedebugrenderer.h"
#include "io/currenttime.h"
#include "io/xmlstream.h"
#include "tool/locklessqueue.h"
#include "projection/projectionsystemsettings.h"
#include "gui/util/frontscene.h"
#include "engine/serverengine.h"
#include "network/netevent.h"

namespace MO {



MO_REGISTER_OBJECT(Scene)

Scene* Scene::p_currentScene_ = 0;

Scene::Scene()
    : Object                  ()
    , p_editor_               (0)
    , p_manager_              (0)
    , p_frontScene_           (0)
    , p_sceneSignals_         (new SceneSignals())
    , p_showSceneDesc_        (false)
    , p_glContext_            (0)
    , p_releaseAllGlRequested_(false)
    , p_fbSize_               (1024,1024)
    , p_fbFormat_             ((int)gl::GL_RGBA)
    , p_fbSizeRequest_        (p_fbSize_)
    , p_fbFormatRequest_      (p_fbFormat_)
    , p_doMatchOutputResolution_(false)
    , p_isShutDown_           (false)
    , p_fboFinal_             (0)
    , p_debugRenderOptions_   (0)
    , p_freeCameraIndex_      (-1)
    , p_freeCameraMatrix_     (1.0)
    , p_projectionSettings_   (new ProjectionSystemSettings())
    , p_projectorIndex_       (0)
    , p_clipController_       (0)
    , p_sceneNumberThreads_   (3)
    , p_sceneSampleRate_      (44100)
    , p_audioCon_             (new AudioObjectConnections())
    , p_isPlayback_           (false)
    , p_lazyFlag_             (false)
    , p_rendering_            (false)
    , p_sceneTime_            (0)
    , p_samplePos_            (0)
{
    MO_DEBUG_TREE("Scene::Scene()");

    setName("Scene");

    p_readWriteLock_ = new QReadWriteLock(QReadWriteLock::Recursive);
}

Scene::~Scene()
{
    MO_DEBUG_TREE("Scene::~Scene()");

    destroyDeletedObjects_(false);

    for (auto i : p_debugRenderer_)
        delete i;
    for (auto i : p_fboFinal_)
        delete i;
    for (auto i : p_screenQuad_)
        delete i;
    delete p_readWriteLock_;
    delete p_audioCon_;
    delete p_projectionSettings_;
    delete p_sceneSignals_;
}

QDataStream& operator<<(QDataStream& io, const std::set<Scene::Locator>& loc)
{
    io << (quint64)loc.size();
    for (auto l : loc)
        io << l.id << l.time;
    return io;
}

QDataStream& operator>>(QDataStream& io, std::set<Scene::Locator>& loc)
{
    quint64 num;
    io >> num;
    loc.clear();
    for (quint64 i=0; i<num; ++i)
    {
        Scene::Locator l;
        io >> l.id >> l.time;
        loc.insert(l);
    }
    return io;
}


void Scene::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("scene", 5);

    // v2
    io << p_fbSize_ << p_doMatchOutputResolution_;

    // v3
    io << p_sceneDesc_ << p_showSceneDesc_;

    // v4
    io << p_locators_;

    // v5
    // changed from map to set
}

void Scene::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    const int ver = io.readHeader("scene", 5);

    if (ver >= 2)
        io >> p_fbSizeRequest_ >> p_doMatchOutputResolution_;

/*#ifdef MO_DISABLE_EXP
    doMatchOutputResolution_ = true;
    //fbSizeRequest_ = QSize(0, 0);
#endif*/

    if (ver >= 3)
        io >> p_sceneDesc_ >> p_showSceneDesc_;

    if (ver == 4)
    {
        QMap<QString, double> loc;
        io >> loc;
        p_locators_.clear();
        for (auto i=loc.begin(); i!=loc.end(); ++i)
            setLocatorTime(i.key(), i.value());
    }
    if (ver >= 5)
        io >> p_locators_;
}


bool Scene::serializeAfterChilds(IO::DataStream & io) const
{
    io.writeHeader("scene_", 2);

    p_audioCon_->serialize(io);

    // v2
#ifndef MO_DISABLE_FRONT
    io << (frontScene_ ? frontScene_->toXml() : QString());
#else
    io << QString();
#endif

    return true;
}

void Scene::deserializeAfterChilds(IO::DataStream & io)
{
    const int ver = io.readHeader("scene_", 2);

    p_audioCon_->deserialize(io, this);

    if (ver>=2)
    {
        io >> p_frontSceneXml_;
    }
}


void Scene::setObjectEditor(ObjectEditor * editor)
{
    p_editor_ = editor;
    p_editor_->setScene(this);
}

void Scene::setCurrentScene(Scene* s)
{
    p_currentScene_ = s;
}

Scene* Scene::currentScene()
{
    return p_currentScene_;
}

void Scene::findObjects_()
{
    MO_DEBUG_TREE("Scene::findObjects_()");

    // all objects including scene
    p_allObjects_ = findChildObjects<Object>(QString(), true);
    p_allObjects_.prepend(this);

    // all cameras
    p_cameras_ = findChildObjects<Camera>(QString(), true);

    // all shader objects
    p_shaderObjects_ = findChildObjects<ShaderObject>(QString(), true);

    // all light sources
    p_lightSources_ = findChildObjects<LightSource>(QString(), true);

    // all objects that need to be rendered
    p_glObjects_ = findChildObjects<ObjectGl>(QString(), true);

    // all objects that draw their frames into the output
    p_frameDrawers_ = findChildObjects<ObjectGl>([](ObjectGl*o)
    {
        return o->isCamera() | o->isShader() | o->isTexture();
    }, true);

    // not all objects need there transformation calculated
    // these are the ones that do
    p_posObjects_ = findChildObjects(TG_REAL_OBJECT, true);

    // assign clip container
    p_clipController_ = 0;
    for (auto o : p_allObjects_)
        if (o->type() == T_CLIP_CONTROLLER)
            p_clipController_ = static_cast<ClipController*>(o);
    // assign clips to it
    if (p_clipController_)
        p_clipController_->collectClips();

    // ui-proxies
    p_uiModsFloat_.clear();
    for (Object * c : p_allObjects_)
    if (c->type() & TG_MODULATOR_OBJECT)
    {
        auto m = dynamic_cast<ModulatorObject*>(c);
        if (!m || !m->isUiProxy())
            continue;

        if (m->type() == T_MODULATOR_OBJECT_FLOAT)
            p_uiModsFloat_.insert(m->uiId(), static_cast<ModulatorObjectFloat*>(m));
    }

#ifdef MO_DO_DEBUG_TREE
    for (auto o : p_allObjects_)
        MO_DEBUG_TREE("object: " << o << ", parent: " << o->parentObject());
#endif

#if (0)
    MO_DEBUG("Scene: " << cameras_.size() << " cameras, "
             << glObjects_.size() << " gl-objects, "
             << audioObjects_.size() << " audio-objects"
             );
#endif
}

void Scene::updateWeakNameLinks()
{
    // render objects per camera
    p_glObjectsPerCamera_.clear();
    for (Camera * c : p_cameras_)
        p_glObjectsPerCamera_ << c->getRenderObjects();

    // XXX strong connections should maybe not be handled here
    // this function is called from updateTree_()
    //dependMap_
}



/*
void Scene::initGlChilds_()
{
    for (auto o : glObjects_)
    {

    }
}
*/

void Scene::render_()
{
    emit sceneSignals()->renderRequest();
}


// ----------------------- files -----------------------------

void Scene::getNeededFiles(IO::FileList &files)
{
    for (auto c : childObjects())
        getNeededFiles_(c, files);
}

void Scene::getNeededFiles_(Object * o, IO::FileList & files)
{
    o->getNeededFiles(files);

    for (auto c : o->childObjects())
        getNeededFiles_(c, files);
}

/** @todo Super inefficient (and not used currently) */
QSet<Object*> Scene::getAllModulators() const
{
    QSet<Object*> set;

    for (auto o : p_allObjects_)
    {
        QList<Object*> list = o->getModulatingObjectsList(true);
        for (auto mod : list)
            set.insert(mod);
    }

    return set;
}

ModulatorObject * Scene::createUiModulator(const QString &uiId)
{
    // return existing?
    for (auto o : childObjects())
        if (auto m = dynamic_cast<ModulatorObject*>(o))
            if (m->uiId() == uiId)
                return m;
    // create
    /** @todo create the appropriate type */
    auto m = ObjectFactory::createModulatorObjectFloat("uiproxy_" + uiId);
    m->setUiId(uiId);
    addObject(this, m);
    return m;
}

QList<ModulatorObject*> Scene::getUiModulatorObjects(const QList<QString>& uiIds) const
{
    // find all modulator objects matching any of the ids
    QList<ModulatorObject*> list;
    for (auto & id : uiIds)
    {
        auto i = p_uiModsFloat_.find(id);
        if (i != p_uiModsFloat_.end())
            list << i.value();
    }

    return list;
}

#if 0
QList<Modulator*> Scene::getUiModulators(const QList<QString>& uiIds) const
{
    // find all modulator objects matching any of the ids
    auto modobjs = getUiModulatorObjects(uiIds);

    //getModulators()
    QList<Modulator*> mods;
    for (ModulatorObject * o : modobjs)
    {
        /*auto i = uiModsFloat_.find(id);
        if (i != uiModsFloat_.end())
            list << i.value();
            */
    }

    return mods;
}
#endif

void Scene::setUiValue(const QString &uiId, Double timeStamp, Float value)
{
    auto i = p_uiModsFloat_.find(uiId);
    if (i == p_uiModsFloat_.end())
    {
        MO_WARNING("Scene::setUiValue(" << uiId << ", " << value << ") "
                   "no such ModulatorObject found");
        return;
    }

#ifndef MO_DISABLE_SERVER
    if (isServer() && serverEngine().isRunning())
    {
        auto e = new NetEventUiFloat;
        e->setValue(uiId, timeStamp, value);
        serverEngine().sendEvent(e);
    }
#endif
    i.value()->setValue(timeStamp, value);
    render_();
}

// ----------------------- tree ------------------------------

void Scene::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_TREE("Scene::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");

    // make the name unique
    newChild->setName( parent->makeUniqueName(newChild->name()) );

    {
        ScopedSceneLockWrite lock(this);

        parent->addObject_(newChild, insert_index);
        // get attached audio cons
        if (auto acon = newChild->getAssignedAudioConnections())
        {
            if (acon->isUnassigned())
                acon->assignPointers(this);
            audioConnections()->addFrom(*acon);
            newChild->assignAudioConnections(0);
        }
        parent->p_childrenChanged_();
        newChild->onParametersLoaded();
        newChild->updateParameterVisibility();
        updateTree_();
    }
    render_();
}

void Scene::addObjects(Object *parent,
                       const QList<Object*>& newChilds,
                       int insert_index)
{
    MO_DEBUG_TREE("Scene::addObjects(" << parent << ", " << newChilds.size() << ", " << insert_index << ")");

    {
        ScopedSceneLockWrite lock(this);

        for (auto n : newChilds)
        {
            // make the name unique
            n->setName( parent->makeUniqueName(n->name()) );
            // add (XXX could be faster with a list version...)
            parent->addObject_(n, insert_index);
            if (insert_index >= 0)
                ++insert_index;

            // get attached audio cons
            if (auto acon = n->getAssignedAudioConnections())
            {
                //acon->dump(std::cout);
                if (acon->isUnassigned())
                    acon->assignPointers(this);
                audioConnections()->addFrom(*acon);
                // remove attachment
                n->assignAudioConnections(0);
            }
        }

        parent->p_childrenChanged_();

        for (auto n : newChilds)
        {
            n->onParametersLoaded();
            n->updateParameterVisibility();
        }
        updateTree_();
    }
    render_();
}

void Scene::deleteObject(Object *object)
{
    MO_DEBUG_TREE("Scene::deleteObject(" << object << ")");

    MO_ASSERT(object->parentObject(), "Scene::deleteObject("<<object<<") without parent");

    QList<Object*> dellist;

    {
        ScopedSceneLockWrite lock(this);

        removeDependencies(object);

        // remove audio connections
        audioConnections()->remove(object);
        //audioConnections()->dump(std::cout);

        // get list of all objects that will be deleted
        dellist = object->findChildObjects<Object>(QString(), true);
        dellist.prepend(object);
        // memorize so we can free resources later
        p_deletedObjects_.append(dellist);
        p_deletedParentObjects_.append(object);

        // get list of all remaining objects
        QList<Object*> remainList = findChildObjectsStopAt<Object>(QString(), true, object);
        remainList.prepend(this);

        // tell everyone about deletions
        tellObjectsAboutToDelete_(remainList, dellist);

        // execute
        Object * parent = object->parentObject();
        parent->deleteObject_(object, false);
        parent->p_childrenChanged_();

        // finally update tree
        updateTree_();
    }

    render_();
}


void Scene::deleteObjects(const QList<Object*>& objects)
{
    MO_DEBUG_TREE("Scene::deleteObjects(" << objects.size() << ")");

    QList<Object*> dellist;

    {
        ScopedSceneLockWrite lock(this);

        for (auto object : objects)
        {
            MO_ASSERT(object->parentObject(), "Scene::deleteObjects(): "<<object<<" without parent");

            removeDependencies(object);

            // remove audio connections
            audioConnections()->remove(object);
            //audioConnections()->dump(std::cout);

            // get list of all objects that will be deleted
            dellist = object->findChildObjects<Object>(QString(), true);
            dellist.prepend(object);
            // memorize so we can free resources later
            p_deletedObjects_.append(dellist);
            p_deletedParentObjects_.append(object);

            // get list of all remaining objects
            QList<Object*> remainList = findChildObjectsStopAt<Object>(QString(), true, object);
            remainList.prepend(this);

            // tell everyone about deletions
            tellObjectsAboutToDelete_(remainList, dellist);

            // execute
            Object * parent = object->parentObject();
            parent->deleteObject_(object, false);
            parent->p_childrenChanged_();
        }

        // finally update tree
        updateTree_();
    }

    render_();
}



bool Scene::setObjectIndex(Object * object, int newIndex)
{
    MO_DEBUG_TREE("Scene::setObjectIndex(" << object << ", " << newIndex << ")");

    auto parent = object->parentObject();
    if (!parent)
        return false;

    if (parent->childObjects().indexOf(object) == newIndex)
        return false;

    {
        ScopedSceneLockWrite lock(this);
        if (!parent->setChildrenObjectIndex_(object, newIndex))
            return false;
        parent->p_childrenChanged_();
        updateTree_();

    }
    render_();
    return true;
}

void Scene::moveObject(Object *object, Object *newParent, int newIndex)
{
    MO_DEBUG_TREE("Scene::moveObject(" << object << ", " << newParent << ", " << newIndex << ")");

    auto oldParent = object->parentObject();
    if (!oldParent)
    {
        addObject(newParent, object, newIndex);
        return;
    }

    {
        ScopedSceneLockWrite lock(this);
        oldParent->p_takeChild_(object);
        newParent->addObject_(object, newIndex);

        oldParent->p_childrenChanged_();
        newParent->p_childrenChanged_();
        updateTree_();
    }
    render_();
}

void Scene::tellObjectsAboutToDelete_(
        const QList<Object *>& toTell, const QList<Object *>& deleted)
{
    for (auto o : toTell)
        o->onObjectsAboutToDelete(deleted);
}

void Scene::updateTree_()
{
    MO_DEBUG_TREE("Scene::updateTree_()");

    //const int numlights = lightSources_.size();

    findObjects_();

    // update debug renderer objects
    for (auto i : p_debugRenderer_)
        if (i)
            i->updateTree();

    // tell all objects if their children have changed
    updateChildrenChanged_();

    // tell everyone how much lights we have
    //if (numlights != lightSources_.size())
    // XXX ^the check is okay, but we need to tell
    // new objects the light sources and that would not happen
        updateNumberLights_();

    // tell all objects how much thread data they need
    updateNumberThreads_();
    updateSampleRate_();

    // collect all modulators for each object
    updateModulators_();

    // get camera render targets mainly
    updateWeakNameLinks();

    // update the rendermodes
    propagateRenderMode(0);

    if (p_glContext_)
    {
        // update infos for new objects
        // XXX This should be iteratively for all glContext_s
        setGlContext(MO_GFX_THREAD, p_glContext_);

        // update image
        render_();
    }
}

void Scene::updateChildrenChanged_()
{
    MO_DEBUG_TREE("Scene::updateChildrenChanged_() ");

    for (auto o : p_allObjects_)
        if (o->haveChildrenChanged())
            o->p_childrenChanged_();
}


void Scene::updateNumberThreads_()
{
    MO_DEBUG_TREE("Scene::updateNumberThreads_() sceneNumberThreads_ == " << p_sceneNumberThreads_);

    if (!verifyNumberThreads(p_sceneNumberThreads_))
        setNumberThreads(p_sceneNumberThreads_);

    for (auto o : p_allObjects_)
        if (!o->verifyNumberThreads(p_sceneNumberThreads_))
            o->setNumberThreads(p_sceneNumberThreads_);
}

void Scene::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);

    uint oldnum = p_fboFinal_.size();
    p_fboFinal_.resize(num);
    p_screenQuad_.resize(num);
    p_lightSettings_.resize(num);
    p_debugRenderer_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        p_fboFinal_[i] = 0;
        p_screenQuad_[i] = 0;
        p_lightSettings_[i].resize(0); // just to be sure
        p_debugRenderer_[i] = 0;
    }
}

void Scene::setSceneSampleRate(uint samplerate)
{
    p_sceneSampleRate_ = samplerate;
    updateSampleRate_();
}

void Scene::updateNumberLights_()
{
    for (auto o : p_glObjects_)
    if ((int)o->numberLightSources() != p_lightSources_.size())
    {
        o->p_numberLightSources_ = p_lightSources_.size();
        // don't notify if objects havn't even been initialized properly
        if (o->numberThreads() == p_sceneNumberThreads_)
            o->numberLightSourcesChanged(MO_GFX_THREAD);
    }
}


void Scene::updateModulators_()
{
    MO_DEBUG_TREE("Scene::updateModulators_()");

    for (auto o : p_allObjects_)
    {
        o->collectModulators();
        // check parameters as well
        for (auto p : o->params()->parameters())
            p->collectModulators();
    }
}


void Scene::updateSampleRate_()
{
    MO_DEBUG_AUDIO("Scene::updateSampleRate_()");

    setSampleRate(p_sceneSampleRate_);

    for (auto o : p_allObjects_)
    {
        o->setSampleRate(p_sceneSampleRate_);
    }
}


// -------------------- parameter ----------------------------

void Scene::notifyParameterVisibility(Parameter *p)
{
    if (p_editor_)
        emit p_editor_->parameterVisibilityChanged(p);
    emit sceneSignals()->parameterVisibilityChanged(p);
}

void Scene::notifyParameterChange(ParameterText * par)
{
    if (!par->object())
        return;

    // check for text parameters
    // XXX They don't exists in that form yet
    /*
    for (Object * o : allObjects_)
    {
        if (o == par->object())
            continue;

        for (Parameter * p : o->params()->parameters())
            if (p->typeName() == "text")
                for (Modulator * m : p->modulators())
                    if (m->modulator() == par->object())
                    {
                        o->onParameterChanged(p);
                        goto next;
                    }
                }
        next:;
    }*/

    // see if anyone depends on this parameter
    auto it = p_dependMap_.lowerBound(par->object());
    while (it != p_dependMap_.end() && it.key() == par->object())
    {
        it.value()->onDependency(par->object());
        ++it;
    }
}

void Scene::installDependency(Object *object, Object *source)
{
    if (p_dependMap_.find(source, object) == p_dependMap_.end())
        p_dependMap_.insert(source, object);
}

void Scene::removeDependency(Object *object, Object *source)
{
    p_dependMap_.remove(source, object);
}

void Scene::removeDependencies(Object *object)
{
    auto copy = p_dependMap_;
    p_dependMap_.clear();
    for (auto i = copy.begin(); i != copy.end(); ++i)
    {
        if (i.value() != object)
            p_dependMap_.insert(i.key(), i.value());
    }
}

// --------------------- tracks ------------------------------

// --------------------- sequence ----------------------------
/*
void Scene::moveSequence(Sequence *seq, Track *from, Track *to)
{
    MO_DEBUG_TREE("Scene::moveSequence('" << seq->idName() << "', '" << from->idName() << "', '"
                  << to->idName() << "'");
    if (seq->track() == to)
    {
        MO_WARNING("duplicated move sequence '" << seq->idName() << "' to track '"
                   << to->idName() << "'");
        return;
    }
    seq->setParentObject(to);
}
*/
void Scene::beginSequenceChange(Sequence * s)
{
    MO_DEBUG_PARAM("Scene::beginSequenceChange(" << s << ")");
    lockWrite_();
    p_changedSequence_ = s;
}

void Scene::endSequenceChange()
{
    MO_DEBUG_PARAM("Scene::endSequenceChange()");
    unlock_();
    if (p_editor_)
        emit p_editor_->sequenceChanged(p_changedSequence_);
    render_();
}

void Scene::beginTimelineChange(Object * o)
{
    MO_DEBUG_PARAM("Scene::beginTimelineChange(" << o << ")");
    lockWrite_();
    p_changedTimelineObject_ = o;
}

void Scene::endTimelineChange()
{
    MO_DEBUG_PARAM("Scene::endTimelineChange()");
    unlock_();
    if (Sequence * s = dynamic_cast<Sequence*>(p_changedTimelineObject_))
        if (p_editor_)
            emit p_editor_->sequenceChanged(s);

    render_();
}

// --------------------- objects -----------------------------

void Scene::beginObjectChange(Object * o)
{
    MO_DEBUG_PARAM("Scene::beginObjectChange(" << o << ")");
    lockWrite_();
    p_changedObject_ = o;
}

void Scene::endObjectChange()
{
    MO_DEBUG_PARAM("Scene::endObjectChange()");
    unlock_();
    if (p_editor_)
        emit p_editor_->objectChanged(p_changedObject_);
    render_();
}

void Scene::destroyDeletedObjects_(bool releaseGl)
{
    // DEBUG
    //MO_PRINT("Scene::destroyDeletedObjects_(" << releaseGl <<")");
#if 0
    for (Object* o : p_deletedObjects_)
        MO_PRINT("deletedObjects: " << o);
#endif
#if 0
    QSet<Object*> unique;
    for (Object* o : p_deletedObjects_)
    {
        if (unique.contains(o))
            MO_WARNING("Object '" << o->idName() << "' is contained "
                       "multiple times in Scene::p_deletedObjects_");
        unique.insert(o);
    }
#endif

    if (releaseGl)
    for (Object * o : p_deletedObjects_)
    {
        //MO_PRINT(":" << o->namePath());

        if (ObjectGl * gl = dynamic_cast<ObjectGl*>(o))
        {
            for (uint i=0; i<gl->numberThreads(); ++i)
                if (gl->isGlInitialized(i))
                    gl->releaseGl(i);
        }
    }

    for (Object * o : p_deletedParentObjects_)
        o->releaseRef("Scene::destroyDeletedObjects");

    p_deletedObjects_.clear();
    p_deletedParentObjects_.clear();
}


// ----------------------- open gl ---------------------------

void Scene::setGlContext(uint thread, GL::Context *context)
{
    MO_DEBUG_GL("Scene::setGlContext(" << thread << ", " << context << ")");

    p_glContext_ = context;
    if (context)
        p_isShutDown_ = false;

    MO_DEBUG_GL("setting gl context for objects");
    for (auto o : p_glObjects_)
        o->p_setGlContext_(thread, p_glContext_);
}

void Scene::createSceneGl_(uint thread)
{
    MO_DEBUG_GL("Scene::createSceneGl_(" << thread << ")");

    p_fboFinal_[thread] = new GL::FrameBufferObject(
                p_fbSize_.width(),
                p_fbSize_.height(),
                gl::GLenum(p_fbFormat_),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                false, false);
    p_fboFinal_[thread]->setName("scene_final");
    p_fboFinal_[thread]->create();
    p_fboFinal_[thread]->unbind();

    // create screen quad
    p_screenQuad_[thread] = new GL::ScreenQuad("scene_quad");
    p_screenQuad_[thread]->setAntialiasing(3);
    p_screenQuad_[thread]->create();

    p_debugRenderer_[thread] = new GL::SceneDebugRenderer(this);
    p_debugRenderer_[thread]->initGl();
    p_debugRenderer_[thread]->updateTree();
}


void Scene::releaseSceneGl_(uint thread)
{
    MO_DEBUG_GL("Scene::releaseSceneGl_(" << thread << ")");

    if (p_fboFinal_[thread])
    {
        p_fboFinal_[thread]->release();
        delete p_fboFinal_[thread];
        p_fboFinal_[thread] = nullptr;
    }

    if (p_screenQuad_[thread])
    {
        p_screenQuad_[thread]->release();
        delete p_screenQuad_[thread];
        p_screenQuad_[thread] = nullptr;
    }

    if (p_debugRenderer_[thread])
    {
        p_debugRenderer_[thread]->releaseGl();
        delete p_debugRenderer_[thread];
        p_debugRenderer_[thread] = nullptr;
    }
}

void Scene::setResolution(const QSize &r)
{
    p_fbSizeRequest_ = r;
    render_();
}

void Scene::resizeFbo_(uint thread)
{
    MO_DEBUG_GL("Scene::resizeFbo_(" << thread << ")");

    if (thread >= p_fboFinal_.size() || !p_fboFinal_[thread])
        return;

    p_fbSize_ = p_fbSizeRequest_;
    p_fbFormat_ = p_fbFormatRequest_;

    if (p_fboFinal_[thread]->isCreated())
        p_fboFinal_[thread]->release();
    delete p_fboFinal_[thread];

    p_fboFinal_[thread] = new GL::FrameBufferObject(
                p_fbSize_.width(),
                p_fbSize_.height(),
                gl::GLenum(p_fbFormat_),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                false, false);
    p_fboFinal_[thread]->setName("scene_final");
    p_fboFinal_[thread]->create();

    emit sceneSignals()->sceneFboChanged();
}

GL::FrameBufferObject * Scene::fboMaster(uint thread) const
{
    if (thread < p_fboFinal_.size())
        return p_fboFinal_[thread];
    else
        return 0;
}

GL::FrameBufferObject * Scene::fboCamera(uint , uint camera_index) const
{
    if ((int)camera_index >= p_cameras_.size())
    {
        MO_WARNING("request for camera fbo " << camera_index
                   << " is out of range (" << p_cameras_.size() << ")");
        return 0;
    }
    return p_cameras_[camera_index]->fbo();
}


/// @todo this is all to be moved out of this class REALLY!

void Scene::renderScene(const RenderTime& time, bool paintToScreen)//, GL::FrameBufferObject * outputFbo)
{
    MO_DEBUG_GL("Scene::renderScene("<<time<<")");

    MO_ASSERT(p_glContext_, "renderScene() without context");

    if (!p_glContext_ || p_frameDrawers_.empty())
        return;

    //Double time = sceneTime_;

    // read-lock is sufficient because we
    // modify only thread-local storage
    ScopedSceneLockRead lock(this);

    try
    {
        // ---------- lazy resource managment -------------

        // free deleted objects resources
        destroyDeletedObjects_(true);

        // release all openGL resources and quit
        if (p_releaseAllGlRequested_)
        {
            releaseSceneGl_(time.thread());

            for (auto o : p_glObjects_)
                if (o->isGlInitialized(time.thread()))
                    o->p_releaseGl_(time.thread());

            p_releaseAllGlRequested_ = false;

            emit sceneSignals()->glReleased(time.thread());
            return;
        }

        // initialize scene gl resources
        if (!p_fboFinal_[time.thread()])
            createSceneGl_(time.thread());

        // resize fbo on request
        if (p_fbSize_ != p_fbSizeRequest_
         || p_fbFormat_ != p_fbFormatRequest_)
            resizeFbo_(time.thread());

        // initialize object gl resources
        for (auto o : p_glObjects_)
        {
            if (o->needsInitGl(time.thread()))// && o->active(time))
            {
                if (o->isGlInitialized(time.thread()))
                    o->p_releaseGl_(time.thread());
                o->p_initGl_(time.thread());
            }
        }

        // --------- render preparation --------------

        // position all objects
        calculateSceneTransform_(time);

        // update lighting uniform-ready data
        updateLightSettings_(time);

    }
    catch (Exception & e)
    {
        e << "\n  in Scene::renderScene(" << time << ") preface";
        throw;
    }


    // --- render ShaderObjects and TextureObjects ----

    if (!p_frameDrawers_.isEmpty())
    {
        GL::RenderSettings renderSet;
        GL::CameraSpace camSpace;

        renderSet.setLightSettings(&lightSettings(time.thread()));
        renderSet.setCameraSpace(&camSpace);
        renderSet.setFinalFramebuffer(p_fboFinal_[time.thread()]);

        try
        {
            uint cindex = 0;

            // for each object that renders
            for (ObjectGl * o : p_frameDrawers_)
            {
                if (o->active(time))
                {
                    // shaders and texture processors
                    if (o->isShader() || o->isTexture())
                    {
                        if (o->updateMode() == ObjectGl::UM_ALWAYS
                            || o->isUpdateRequest()
                            || o->params()->haveInputsChanged(time))
                        {
                            o->p_renderGl_(renderSet, time);
                        }
                    }

                    // camera
                    else if (o->isCamera())
                    {
                        Camera * camera = static_cast<Camera*>(o);

                        // skip camera?
                        /*if (camera->updateMode() == ObjectGl::UM_ON_CHANGE
                            && !camera->isUpdateRequest())
                            continue;*/

                        // update the list of to-render objects
                        if (camera->needsUpdateRenderObjects())
                            p_glObjectsPerCamera_[cindex] = camera->getRenderObjects();

                        // get camera viewspace
                        camera->initCameraSpace(camSpace, time);

                        // get camera view-matrix
                        const Mat4& cammat = camera->transformation();
                        camSpace.setPosition(Vec3(cammat[3][0], cammat[3][1], cammat[3][2]));
                        const Mat4 viewm = glm::inverse(cammat);
                        camSpace.setViewMatrix(viewm);

// Render all objects per cube-face
// This version seems *slightly* faster than the one below
#if 1
                        // for each cubemap
                        const uint numCubeMaps = camera->numCubeTextures(time);
                        for (uint i=0; i<numCubeMaps; ++i)
                        {
                            // start camera frame
                            camera->startGlFrame(time, i);

                            camSpace.setCubeViewMatrix( camera->cameraViewMatrix(i) * viewm );

                            // render each opengl object per camera & per cube-face
                            for (ObjectGl * o : p_glObjectsPerCamera_[cindex])
                            if (!o->isShader() && !o->isTexture() // don't render shader objects per camera
                                && o->active(time))
                            {
                                // XXX Not working for cube-maps
                                if (o->updateMode() == ObjectGl::UM_ON_CHANGE
                                    && !(o->isUpdateRequest() || o->params()->haveInputsChanged(time)))
                                        continue;

                                o->p_renderGl_(renderSet, time);
                            }

                            // render debug objects
                            if (p_debugRenderOptions_)
                                p_debugRenderer_[time.thread()]
                                        ->render(renderSet, time.thread(), p_debugRenderOptions_);
                        }

                        camera->finishGlFrame(time);

// Render each cube-face per object
#else
                        bool isFirst = true;
                        // render each opengl object per camera & per cube-face
                        for (ObjectGl * o : p_glObjectsPerCamera_[cindex])
                        if (!o->isShader() && !o->isTexture() // don't render shader objects per camera
                            && o->active(time))
                        {
                            if (o->updateMode() == ObjectGl::UM_ON_CHANGE
                                && !(o->isUpdateRequest() || o->params()->haveInputsChanged(time)))
                                    continue;

                            // for each cubemap
                            const uint numCubeMaps = camera->numCubeTextures(time);
                            for (uint i=0; i<numCubeMaps; ++i)
                            {
                                // start camera frame
                                // and attach correct render target texture
                                if (isFirst)
                                    camera->startGlFrame(time, i);
                                else
                                    camera->attachCubeTexture(i);

                                camSpace.setCubeViewMatrix( camera->cameraViewMatrix(i) * viewm );

                                o->p_renderGl_(renderSet, time);
                            }
                            isFirst = false;

                            //YYY render debug objects
                            /*if (p_debugRenderOptions_)
                                p_debugRenderer_[time.thread()]
                                        ->render(renderSet, time.thread(), p_debugRenderOptions_);
                                        */
                        }

                        camera->finishGlFrame(time);
#endif
                    }

                } // active

                if (o->isCamera())
                    ++cindex;
            }
        }
        catch (Exception & e)
        {
            e << "\n  in Scene::renderScene(" << time.thread() << "): frame-drawers";
            throw;
        }
    }

    // okay, we've painted them
    for (ObjectGl * o : p_glObjects_)
        o->clearUpdateRequest();

//    MO_DEBUG("render finalFbo, time == " << time << ", cameras_.size() == " << cameras_.size());    

    // --- mix camera frames and framedrawers ---

    using namespace gl;

    p_fboFinal_[time.thread()]->bind();
    p_fboFinal_[time.thread()]->setViewport();
    MO_CHECK_GL( glClearColor(0, 0, 0, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( glDisable(GL_DEPTH_TEST) );
    for (ObjectGl * drawer : p_frameDrawers_)
    if (drawer->active(time))
    {
        if (drawer->isCamera())
            static_cast<Camera*>(drawer)->drawFramebuffer(time);
        else if (drawer->isShader())
            static_cast<ShaderObject*>(drawer)->drawFramebuffer(time,
                                                                p_fboFinal_[time.thread()]->width(),
                                                                p_fboFinal_[time.thread()]->height());
        else if (drawer->isTexture())
            static_cast<TextureObjectBase*>(drawer)->drawFramebuffer(time,
                                                                p_fboFinal_[time.thread()]->width(),
                                                                p_fboFinal_[time.thread()]->height());
    }
    p_fboFinal_[time.thread()]->unbind();

    // --- draw to screen ---

    if (!paintToScreen)
        return;

    //MO_DEBUG_GL("Scene::renderScene(" << thread << ")");

    int width = p_glContext_->size().width(),
        height = p_glContext_->size().height();
    // .. or output fbo
    /*
    if (outputFbo)
    {
        width = outputFbo->width();
        height = outputFbo->height();

        outputFbo->bind();
    }*/
    //width = fbSize_.width();
    //height = fbSize_.height();

    p_fboFinal_[time.thread()]->colorTexture()->bind();
    MO_CHECK_GL( glViewport(0, 0, width, height) );
    MO_CHECK_GL( glClearColor(0.1, 0.1, 0.1, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( glDisable(GL_BLEND) );
    if (isClient())
        p_screenQuad_[time.thread()]->draw(width, height);
    else
        p_screenQuad_[time.thread()]->drawCentered(width, height,
                                                 p_fboFinal_[time.thread()]->aspect());

    //if (outputFbo)
    //    outputFbo->unbind();
}

void Scene::updateLightSettings_(const RenderTime& time)
{
    MO_ASSERT(time.thread() < p_lightSettings_.size(), "thread " << time.thread() << " for "
              "LightSettings out-of-range (" << p_lightSettings_.size() << ")");

    GL::LightSettings * l = &p_lightSettings_[time.thread()];

    // resize if necessary
    if ((int)l->count() != p_lightSources_.size())
        l->resize(p_lightSources_.size());

    // fill vectors
    for (uint i=0; i<l->count(); ++i)
    {
        if (p_lightSources_[i]->active(time))
        {
            p_lightSources_[i]->getLightSettings(l, i, time);
        }
        else
            // XXX there should be a runtime switch in shader!
            l->setColor(i, 0,0,0,1);
    }
}

void Scene::calculateSceneTransform(const RenderTime& time)
{
    ScopedSceneLockRead lock(this);
    calculateSceneTransform_(time);
}

void Scene::calculateSceneTransform_(const RenderTime& time)
{
#if 0
    // interpolate free camera
    if (freeCameraIndex_ >= 0)
    {
        freeCameraMatrixGfx_ = glm::inverse(freeCameraMatrix_);
        //freeCameraMatrixGfx_ += (Float)1 / 30 *
        //        (glm::inverse(freeCameraMatrix_) - freeCameraMatrixGfx_);
    }
#endif

    // init root matrix for all other objects below scene
    clearTransformation();

//    int camcount = 0;

    // calculate transformations
    for (auto &o : p_posObjects_)
    {
#if 0
        // see if this object is a user-controlled camera
        bool freecam = false;
        if (o->isCamera())
        {
            if (camcount == freeCameraIndex_)
                freecam = true;
            ++camcount;
        }
#endif
        if (o->active(time))
        {
//            if (!freecam)
//            {
                // get parent transformation
                Mat4 matrix(o->parentObject()->transformation());
                // apply object's transformation
                o->calculateTransformation(matrix, time);
                // write back
                o->setTransformation(matrix);
//            }
//            else
//                o->setTransformation(freeCameraMatrixGfx_);
        }
    }
}

void Scene::setFreeCameraIndex(int index)
{
    p_freeCameraIndex_ = index;
    for (auto c : p_cameras_)
        c->clearOverrideMatrix();
    if (p_freeCameraIndex_ >= 0 && p_freeCameraIndex_ < p_cameras_.size())
        p_cameras_[p_freeCameraIndex_]->setOverrideMatrix(p_freeCameraMatrix_);
    render_();
}

void Scene::setFreeCameraMatrix(const MO::Mat4& mat)
{
    p_freeCameraMatrix_ = glm::inverse(mat);
    if (p_freeCameraIndex_ < 0 || p_freeCameraIndex_ >= p_cameras_.size())
        return;

    p_cameras_[p_freeCameraIndex_]->setOverrideMatrix(p_freeCameraMatrix_);

    render_();
}




// ---------------------- runtime --------------------------

void Scene::runScripts()
{
    for (Object * o : p_allObjects_)
    if (o->activeAtAll())
    if (auto script = dynamic_cast<AScriptObject*>(o))
    {
        try
        {
            script->runScript();
        }
        catch (const Exception & e)
        {
            /** @todo put this in a log console, visible to user! */
            MO_WARNING("The script object '" << script->name() << "' failed:\n"
                       << e.what());
        }
    }
}

void Scene::lockRead_()
{
    p_readWriteLock_->lockForRead();
}

void Scene::lockWrite_()
{
    p_readWriteLock_->lockForWrite();
}

void Scene::unlock_()
{
    p_readWriteLock_->unlock();
}

void Scene::destroyGlRequest()
{
    MO_DEBUG_GL("Scene::destroyGlRequest()");

    if (!p_glContext_)
        return;

    // release opengl resources later in their thread
    p_releaseAllGlRequested_ = true;

    // move to later (Window must repaint!)
    render_();
}

void Scene::destroyGlNow()
{
    MO_DEBUG_GL("Scene::destroyGlNow()");

    if (!p_glContext_)
        return;

    p_releaseAllGlRequested_ = true;

    // kill now (The calling thread must be GFX thread!)
    p_glContext_->makeCurrent();
    renderScene(RenderTime(sceneTime(), MO_GFX_THREAD));
    p_isShutDown_ = true;
}

void Scene::setSceneTime(Double time, bool send_signal)
{
    p_sceneTime_ = time;
    p_samplePos_ = time * sampleRate();

    CurrentTime::setTime(time);

    if (send_signal)
        emit sceneSignals()->sceneTimeChanged(p_sceneTime_);

//    render_();
}

void Scene::setSceneTime(SamplePos pos, bool send_signal)
{
    p_sceneTime_ = pos * sampleRateInv();
    p_samplePos_ = pos;

    CurrentTime::setTime(p_sceneTime_);

    if (send_signal)
        emit sceneSignals()->sceneTimeChanged(p_sceneTime_);
//    render_();
}


void Scene::setProjectionSettings(const ProjectionSystemSettings & p)
{
    *p_projectionSettings_ = p;

    // update all cameras
    for (Camera * c : p_cameras_)
        for (uint i=0; i<c->numberThreads(); ++i)
            c->p_needsInitGl_[i] = true;

    render_();
}

void Scene::setProjectorIndex(uint index)
{
    if (index == p_projectorIndex_)
        return;

    p_projectorIndex_ = index;

    if (index < p_projectionSettings_->numProjectors())
        p_fbSizeRequest_ = QSize(
                    p_projectionSettings_->cameraSettings(index).width(),
                    p_projectionSettings_->cameraSettings(index).height());

    // update all cameras
    for (Camera * c : p_cameras_)
        for (uint i=0; i<c->numberThreads(); ++i)
            c->p_needsInitGl_[i] = true;

    render_();
}



double Scene::locatorTime(const QString& id) const
{
    for (auto i = p_locators_.begin(); i != p_locators_.end(); ++i)
        if (i->id == id)
            return i->time;
    return 0.;
    /*
    auto i = p_locators_.find(id);
    return i == p_locators_.end() ? 0. : i.value();
    */
}

void Scene::setLocatorTime(const QString& id, double t)
{
    for (auto i = p_locators_.begin(); i!= p_locators_.end(); ++i)
        if (i->id == id)
            { p_locators_.erase(i); break; }
    Locator l;
    l.id = id;
    l.time = t;
    p_locators_.insert(l);
}

void Scene::renameLocator(const QString& id, const QString& newId)
{
#if 1
    for (auto i = p_locators_.begin(); i != p_locators_.end(); ++i)
    if (i->id == id)
    {
        i->id == newId;
        return;
    }

#else
    auto i = p_locators_.find(id);
    if (i == p_locators_.end())
        return;
    double t = i.value();
    p_locators_.erase(i);
    p_locators_.insert(newId, t);
#endif
}

void Scene::deleteLocator(const QString& id)
{
#if 1
    for (auto i = p_locators_.begin(); i != p_locators_.end(); ++i)
        if (i->id == id)
            { p_locators_.erase(i); return; }
#else
    p_locators_.remove(id);
#endif
}


void Scene::insertTime(Double where, Double howMuch, bool emitSignals)
{
    // change sequences
    auto seqs = findChildObjects<Sequence>(QString(), true);
    for (Sequence* s : seqs)
    {
        if (s->start() >= where)
            s->setStart(s->start() + howMuch);
    }

    // adjust locators
    auto loc = p_locators_;
    p_locators_.clear();
    for (auto i = loc.begin(); i!=loc.end(); ++i)
    {
        setLocatorTime(i->id,
                      (i->time >= where)
                            ? (i->time + howMuch)
                            :  i->time );
    }

    if (emitSignals)
    if (auto e = editor())
    for (Sequence* s : seqs)
        emit e->sequenceChanged(s);

}


} // namespace MO
