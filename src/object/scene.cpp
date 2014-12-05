/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>
#include <QReadWriteLock>

#include "scene.h"
#include "scenelock_p.h"
#include "camera.h"
#include "io/error.h"
#include "io/log.h"
#include "io/datastream.h"
#include "object/objectfactory.h"
#include "object/param/parameters.h"
#include "object/param/parameterint.h"
#include "object/param/parameterfilename.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/param/parametertext.h"
#include "object/param/parametertimeline1d.h"
#include "object/track.h"
#include "object/sequencefloat.h"
#include "object/microphone.h"
#include "object/lightsource.h"
#include "object/util/objectdsppath.h"
#include "object/util/objecteditor.h"
#include "object/util/audioobjectconnections.h"
#include "audio/audiodevice.h"
#include "audio/audiosource.h"
#include "gl/context.h"
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "gl/screenquad.h"
#include "gl/texture.h"
#include "gl/rendersettings.h"
#include "gl/scenedebugrenderer.h"
#include "tool/locklessqueue.h"
#include "io/currenttime.h"
#include "projection/projectionsystemsettings.h"

namespace MO {



MO_REGISTER_OBJECT(Scene)

Scene::Scene(QObject *parent) :
    Object              (parent),
    editor_             (0),
    glContext_          (0),
    releaseAllGlRequested_(0),
    fbSize_             (1024, 1024),
    fbFormat_           ((int)gl::GL_RGBA),
    fbSizeRequest_      (fbSize_),
    fbFormatRequest_    (fbFormat_),
    doMatchOutputResolution_(false),
    isShutDown_         (false),
    fboFinal_           (0),
    debugRenderOptions_ (0),
    freeCameraIndex_    (-1),
    freeCameraMatrix_   (1.0),
    projectionSettings_ (new ProjectionSystemSettings()),
    projectorIndex_     (0),
    sceneNumberThreads_ (3),
    sceneSampleRate_    (44100),
    audioCon_           (new AudioObjectConnections()),
    isPlayback_         (false),
    sceneTime_          (0),
    samplePos_          (0)
{
    MO_DEBUG_TREE("Scene::Scene()");

    setName("Scene");

    readWriteLock_ = new QReadWriteLock(QReadWriteLock::Recursive);

    sceneBufferSize_.resize(sceneNumberThreads_);
    releaseAllGlRequested_.resize(sceneNumberThreads_);
    sceneBufferSize_[MO_GUI_THREAD] =
    sceneBufferSize_[MO_GFX_THREAD] = 1;
    sceneBufferSize_[MO_AUDIO_THREAD] = 32;

    sceneDelaySize_.resize(sceneNumberThreads_);
    sceneDelaySize_[MO_GUI_THREAD] =
    sceneDelaySize_[MO_GFX_THREAD] = 0;
    sceneDelaySize_[MO_AUDIO_THREAD] = nextPowerOfTwo(96000);
}

Scene::~Scene()
{
    MO_DEBUG_TREE("Scene::~Scene()");

    destroyDeletedObjects_(false);

    for (auto i : debugRenderer_)
        delete i;
    for (auto i : fboFinal_)
        delete i;
    for (auto i : screenQuad_)
        delete i;
    delete readWriteLock_;
    delete audioCon_;
    delete projectionSettings_;
}

void Scene::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("scene", 2);

    // v2
    io << fbSize_ << doMatchOutputResolution_;
}

bool Scene::serializeAfterChilds(IO::DataStream & io) const
{
    io.writeHeader("scene_", 1);

    audioCon_->serialize(io);
    return true;
}

void Scene::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    const int ver = io.readHeader("scene", 2);

    if (ver >= 2)
        io >> fbSizeRequest_ >> doMatchOutputResolution_;
}

void Scene::deserializeAfterChilds(IO::DataStream & io)
{
    io.readHeader("scene_", 1);

    audioCon_->deserialize(io, this);
}


void Scene::setObjectEditor(ObjectEditor * editor)
{
    editor_ = editor;
    editor_->setScene(this);
}

void Scene::findObjects_()
{
    MO_DEBUG_TREE("Scene::findObjects_()");

    // all objects including scene
    allObjects_ = findChildObjects<Object>(QString(), true);
    allObjects_.prepend(this);

    // all cameras
    cameras_ = findChildObjects<Camera>(QString(), true);

    // all light sources
    lightSources_ = findChildObjects<LightSource>(QString(), true);

    // all objects that need to be rendered
    glObjects_ = findChildObjects<ObjectGl>(QString(), true);

    // not all objects need there transformation calculated
    // these are the ones that do
    posObjects_ = findChildObjects(TG_REAL_OBJECT, true);
#if 0
    // all transformation objects that are or include audio relevant objects
    posObjectsAudio_.clear();
    for (auto o : posObjects_)
    {
        if (o->isAudioRelevant())
            posObjectsAudio_.append(o);
    }

    // all objects with audio sources
    audioObjects_.clear();
    for (auto o : allObjects_)
        if (!o->audioSources().isEmpty())
        {
            audioObjects_.append(o);
            allAudioSources_.append( o->audioSources() );
        }

    // all objects with microphones
    numMicrophones_ = 0;
    microphoneObjects_.clear();
    for (auto o : allObjects_)
        if (!o->microphones().isEmpty())
        {
            microphoneObjects_.append(o);
            numMicrophones_ += o->microphones().size();
        }

    // toplevel audio units
    topLevelAudioUnits_ = findChildObjects<AudioUnit>(QString(), false);
#endif

#ifdef MO_DO_DEBUG_TREE
    for (auto o : allObjects_)
        MO_DEBUG_TREE("object: " << o << ", parent: " << o->parentObject());
#endif

#if (0)
    MO_DEBUG("Scene: " << cameras_.size() << " cameras, "
             << glObjects_.size() << " gl-objects, "
             << audioObjects_.size() << " audio-objects"
             );
#endif
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
    emit renderRequest();
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

QSet<Object*> Scene::getAllModulators() const
{
    QSet<Object*> set;

    for (auto o : allObjects_)
    {
        QList<Object*> list = o->getModulatingObjects();
        for (auto mod : list)
            set.insert(mod);
    }

    return set;
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
        parent->childrenChanged_();
        newChild->onParametersLoaded();
        newChild->updateParameterVisibility();
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

        // remove audio connections
        audioConnections()->remove(object);
        //audioConnections()->dump(std::cout);

        // get list of all objects that will be deleted
        dellist = object->findChildObjects<Object>(QString(), true);
        dellist.prepend(object);

        // get list of all remaining objects
        QList<Object*> remainList = findChildObjectsStopAt<Object>(QString(), true, object);
        remainList.prepend(this);

        // tell everyone about deletions
        tellObjectsAboutToDelete_(remainList, dellist);

        // execute
        Object * parent = object->parentObject();
        parent->deleteObject_(object, false);
        parent->childrenChanged_();

        // finally update tree
        updateTree_();
    }

    // memorize so we can free resources later
    deletedObjects_.append(dellist);

    // XXX right now GUI does not listen to the specific
    // object but rather updates everything.
    // So rather not call this repeatedly.
    //for (auto o : dellist)
        //emit objectDeleted(o);

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
        parent->childrenChanged_();
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
        oldParent->takeChild_(object);
        newParent->addObject_(object, newIndex);

        oldParent->childrenChanged_();
        newParent->childrenChanged_();
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

void Scene::callCreateOutputs_(Object *o)
{
    // keep list of childs
    QList<Object*> list = o->childObjects();

    o->createOutputs();

    // emit changes
    if (editor_)
        for (auto c : o->childObjects())
            if (!list.contains(c))
                emit editor_->objectAdded(c);
}

void Scene::callCreateAudioSources_(Object *o)
{
    const QList<AUDIO::AudioSource*> before = o->audioSources();

    o->createAudioSources();

    if (o->audioSources() != before)
    {
        updateTree_();
    }
}

void Scene::callCreateMicrophones_(Object *o)
{
    const QList<AUDIO::AudioMicrophone*> before = o->microphones();

    o->createMicrophones();

    if (o->microphones() != before)
    {
        updateTree_();
    }
}

void Scene::updateTree_()
{
    MO_DEBUG_TREE("Scene::updateTree_()");

    const int numlights = lightSources_.size();

    findObjects_();

    // update debug renderer objects
    for (auto i : debugRenderer_)
        if (i)
            i->updateTree();

    // tell all objects if their children have changed
    updateChildrenChanged_();

    // tell everyone how much lights we have
    if (numlights != lightSources_.size())
        updateNumberLights_();

    // tell all objects how much thread data they need
    updateNumberThreads_();
    updateBufferSize_();
    updateSampleRate_();

    // collect all modulators for each object
    updateModulators_();

    // XXX testing here
    // create the audio-dsp graph
    ObjectDspPath path;
    path.createPath(this, AUDIO::Configuration(sampleRate(), 4096, 0, 2));
    path.dump(std::cout);

    // update the rendermodes
    propagateRenderMode(0);

    if (glContext_)
    {
        // update infos for new objects
        // XXX This should be iteratively for all glContext_s
        setGlContext(MO_GFX_THREAD, glContext_);

        // update image
        render_();
    }
}
void Scene::updateChildrenChanged_()
{
    MO_DEBUG_TREE("Scene::updateChildrenChanged_() ");

    for (auto o : allObjects_)
        if (o->childrenHaveChanged_)
            o->childrenChanged_();
}

void Scene::updateNumberThreads_()
{
    MO_DEBUG_TREE("Scene::updateNumberThreads_() sceneNumberThreads_ == " << sceneNumberThreads_);

    if (!verifyNumberThreads(sceneNumberThreads_))
        setNumberThreads(sceneNumberThreads_);

    for (auto o : allObjects_)
        if (!o->verifyNumberThreads(sceneNumberThreads_))
            o->setNumberThreads(sceneNumberThreads_);
}

void Scene::setNumberThreads(uint num)
{
    Object::setNumberThreads(num);

    uint oldnum = fboFinal_.size();
    fboFinal_.resize(num);
    screenQuad_.resize(num);
    lightSettings_.resize(num);
    debugRenderer_.resize(num);
    freeCameraMatrixAudio_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        fboFinal_[i] = 0;
        screenQuad_[i] = 0;
        lightSettings_[i].resize(0); // just to be sure
        debugRenderer_[i] = 0;
    }
}

void Scene::updateNumberLights_()
{
    for (auto o : glObjects_)
    if ((int)o->numberLightSources() != lightSources_.size())
    {
        o->p_numberLightSources_ = lightSources_.size();
        // don't notify if objects havn't even been initialized properly
        if (o->numberThreads() == sceneNumberThreads_)
            o->numberLightSourcesChanged(MO_GFX_THREAD);
    }
}

void Scene::updateBufferSize_()
{
    MO_DEBUG_AUDIO("Scene::updateBufferSize_() numberThreads() == " << numberThreads());

    for (uint i=0; i<sceneNumberThreads_; ++i)
        if (!verifyBufferSize(i, sceneBufferSize_[i]))
            setBufferSize(sceneBufferSize_[i], i);

#ifdef MO_DO_DEBUG_AUDIO
    for (uint i=0; i<sceneNumberThreads_; ++i)
        MO_DEBUG_AUDIO("bufferSize("<<i<<") == " << bufferSize(i));
#endif

    for (auto o : allObjects_)
    {
        for (uint i=0; i<sceneNumberThreads_; ++i)
        {
            if (!o->verifyBufferSize(i, sceneBufferSize_[i]))
                o->setBufferSize(sceneBufferSize_[i], i);
        }
    }
}


void Scene::updateModulators_()
{
    MO_DEBUG_TREE("Scene::updateModulators_()");

    for (auto o : allObjects_)
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

    setSampleRate(sceneSampleRate_);

    for (auto o : allObjects_)
    {
        o->setSampleRate(sceneSampleRate_);
    }
}


// -------------------- parameter ----------------------------


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
    changedSequence_ = s;
}

void Scene::endSequenceChange()
{
    MO_DEBUG_PARAM("Scene::endSequenceChange()");
    unlock_();
    if (editor_)
        emit editor_->sequenceChanged(changedSequence_);
    render_();
}

void Scene::beginTimelineChange(Object * o)
{
    MO_DEBUG_PARAM("Scene::beginTimelineChange(" << o << ")");
    lockWrite_();
    changedTimelineObject_ = o;
}

void Scene::endTimelineChange()
{
    MO_DEBUG_PARAM("Scene::endTimelineChange()");
    unlock_();
    if (Sequence * s = qobject_cast<Sequence*>(changedTimelineObject_))
        if (editor_)
            emit editor_->sequenceChanged(s);

    render_();
}

// --------------------- objects -----------------------------

void Scene::beginObjectChange(Object * o)
{
    MO_DEBUG_PARAM("Scene::beginObjectChange(" << o << ")");
    lockWrite_();
    changedObject_ = o;
}

void Scene::endObjectChange()
{
    MO_DEBUG_PARAM("Scene::endObjectChange()");
    unlock_();
    if (editor_)
        emit editor_->objectChanged(changedObject_);
    render_();
}

void Scene::destroyDeletedObjects_(bool releaseGl)
{
    for (Object * o : deletedObjects_)
    {
        // unmake the QObject tree structure so we can
        // destroy each object safely
        o->setParent(0);

        if (releaseGl)
        if (ObjectGl * gl = qobject_cast<ObjectGl*>(o))
        {
            for (uint i=0; i<gl->numberThreads(); ++i)
                if (gl->isGlInitialized(i))
                    gl->releaseGl(i);
        }
    }

    for (Object * o : deletedObjects_)
        delete o;

    deletedObjects_.clear();
}


// ----------------------- open gl ---------------------------

void Scene::setGlContext(uint thread, GL::Context *context)
{
    MO_DEBUG_GL("Scene::setGlContext(" << thread << ", " << context << ")");

    glContext_ = context;

    MO_DEBUG_GL("setting gl context for objects");
    for (auto o : glObjects_)
        o->p_setGlContext_(thread, glContext_);
}

void Scene::createSceneGl_(uint thread)
{
    MO_DEBUG_GL("Scene::createSceneGl_(" << thread << ")");

    fboFinal_[thread] = new GL::FrameBufferObject(
                fbSize_.width(),
                fbSize_.height(),
                gl::GLenum(fbFormat_),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                false, GL::ER_THROW);
    fboFinal_[thread]->create();
    fboFinal_[thread]->unbind();

    // create screen quad
    screenQuad_[thread] = new GL::ScreenQuad("scene_quad", GL::ER_THROW);
    screenQuad_[thread]->setAntialiasing(3);
    screenQuad_[thread]->create();

    debugRenderer_[thread] = new GL::SceneDebugRenderer(this);
    debugRenderer_[thread]->initGl();
    debugRenderer_[thread]->updateTree();
}


void Scene::releaseSceneGl_(uint thread)
{
    MO_DEBUG_GL("Scene::releaseSceneGl_(" << thread << ")");

    fboFinal_[thread]->release();
    delete fboFinal_[thread];
    fboFinal_[thread] = 0;

    screenQuad_[thread]->release();
    delete screenQuad_[thread];
    screenQuad_[thread] = 0;

    debugRenderer_[thread]->releaseGl();
    delete debugRenderer_[thread];
    debugRenderer_[thread] = 0;
}

void Scene::setResolution(const QSize &r)
{
    fbSizeRequest_ = r;
    render_();
}

void Scene::resizeFbo_(uint thread)
{
    MO_DEBUG_GL("Scene::resizeFbo_(" << thread << ")");

    if (thread >= fboFinal_.size() || !fboFinal_[thread])
        return;

    fbSize_ = fbSizeRequest_;
    fbFormat_ = fbFormatRequest_;

    if (fboFinal_[thread]->isCreated())
        fboFinal_[thread]->release();
    delete fboFinal_[thread];

    fboFinal_[thread] = new GL::FrameBufferObject(
                fbSize_.width(),
                fbSize_.height(),
                gl::GLenum(fbFormat_),
                gl::GL_FLOAT,
                GL::FrameBufferObject::A_DEPTH,
                false,
                GL::ER_THROW);
    fboFinal_[thread]->create();

    emit sceneFboChanged();
}

GL::FrameBufferObject * Scene::fboMaster(uint thread) const
{
    if (thread < fboFinal_.size())
        return fboFinal_[thread];
    else
        return 0;
}

GL::FrameBufferObject * Scene::fboCamera(uint thread, uint camera_index) const
{
    if ((int)camera_index >= cameras_.size())
    {
        MO_WARNING("request for camera fbo " << camera_index
                   << " is out of range (" << cameras_.size() << ")");
        return 0;
    }
    return cameras_[camera_index]->fbo(thread);
}

void Scene::renderScene(Double time, uint thread, GL::FrameBufferObject * outputFbo)
{
    //MO_DEBUG_GL("Scene::renderScene("<<time<<", "<<thread<<")");

    MO_ASSERT(glContext_, "renderScene() without context");

    if (!glContext_ || cameras_.empty())
        return;

    //Double time = sceneTime_;

    try
    {
        // read-lock is sufficient because we
        // modify only thread-local storage
        ScopedSceneLockRead lock(this);

        // free deleted objects resources
        destroyDeletedObjects_(true);

        // release all openGL resources and quit
        if (releaseAllGlRequested_[thread])
        {
            releaseSceneGl_(thread);

            for (auto o : glObjects_)
                if (o->isGlInitialized(thread))
                    o->p_releaseGl_(thread);

            releaseAllGlRequested_[thread] = false;

            emit glReleased(thread);
            return;
        }

        // initialize scene gl resources
        if (!fboFinal_[thread])
            createSceneGl_(thread);

        // resize fbo on request
        if (fbSize_ != fbSizeRequest_
         || fbFormat_ != fbFormatRequest_)
            resizeFbo_(thread);

        // initialize object gl resources
        for (auto o : glObjects_)
            if (o->needsInitGl(thread))// && o->active(time, thread))
            {
                if (o->isGlInitialized(thread))
                    o->p_releaseGl_(thread);
                o->p_initGl_(thread);
            }

        // position all objects
        calculateSceneTransform(thread, 0, time);

        // update lighting settings
        updateLightSettings_(thread, time);

        GL::RenderSettings renderSet;
        GL::CameraSpace camSpace;

        renderSet.setLightSettings(&lightSettings(thread));
        renderSet.setCameraSpace(&camSpace);
        renderSet.setFinalFramebuffer(fboFinal_[thread]);

        // render scene from each camera
        for (auto camera : cameras_)
        if (camera->active(time, thread))
        {
            // get camera viewspace
            camera->initCameraSpace(camSpace, thread, time);

            // get camera view-matrix
            const Mat4 viewm = glm::inverse(camera->transformation(thread, 0));
            camSpace.setViewMatrix(viewm);

            // for each cubemap
            const uint numCubeMaps = camera->numCubeTextures(thread, time);
            for (uint i=0; i<numCubeMaps; ++i)
            {
                // start camera frame
                camera->startGlFrame(thread, time, i);

                camSpace.setCubeViewMatrix( camera->cameraViewMatrix(i) * viewm );

                // render all opengl objects
                for (auto o : glObjects_)
                if (o->active(time, thread))
                {
                    o->p_renderGl_(renderSet, thread, time);
                }

                // render debug objects
                if (debugRenderOptions_)
                    debugRenderer_[thread]->render(renderSet, thread, debugRenderOptions_);
            }

            camera->finishGlFrame(thread, time);

        }
    }
    catch (Exception & e)
    {
        e << "\nin Scene::renderScene(" << thread << ")";
        throw;
    }


    using namespace gl;

    // --- mix camera frames ---

    fboFinal_[thread]->bind();
    fboFinal_[thread]->setViewport();
    MO_CHECK_GL( glClearColor(0, 0, 0, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( glDisable(GL_DEPTH_TEST) );
    for (auto camera : cameras_)
        if (camera->active(time, thread))
            camera->drawFramebuffer(thread, time);
    fboFinal_[thread]->unbind();

    // --- draw to screen ---

    //MO_DEBUG_GL("Scene::renderScene(" << thread << ")");

    int width = glContext_->size().width(),
        height = glContext_->size().height();
    // .. or output fbo
    if (outputFbo)
    {
        width = outputFbo->width();
        height = outputFbo->height();

        outputFbo->bind();
    }

    fboFinal_[thread]->colorTexture()->bind();
    MO_CHECK_GL( glViewport(0, 0, width, height) );
    MO_CHECK_GL( glClearColor(0.1, 0.1, 0.1, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( glDisable(GL_BLEND) );
    screenQuad_[thread]->drawCentered(width, height, fboFinal_[thread]->aspect());
    fboFinal_[thread]->colorTexture()->unbind();

    if (outputFbo)
        outputFbo->unbind();
}

void Scene::updateLightSettings_(uint thread, Double time)
{
    MO_ASSERT(thread < lightSettings_.size(), "thread " << thread << " for "
              "LightSettings out-of-range (" << lightSettings_.size() << ")");

    GL::LightSettings * l = &lightSettings_[thread];

    // resize if necessary
    if ((int)l->count() != lightSources_.size())
        l->resize(lightSources_.size());

    // fill vectors
    for (uint i=0; i<l->count(); ++i)
    {
        if (lightSources_[i]->active(time, thread))
        {
            const Vec3 pos = lightSources_[i]->position(thread, 0);
            l->setPosition(i, pos[0], pos[1], pos[2]);

            const Vec4 col = lightSources_[i]->lightColor(thread, time);
            l->setColor(i, col[0], col[1], col[2], col[3]);

            const Float mix = lightSources_[i]->lightDirectionalMix(thread, time);
            l->setDirectionMix(i, mix);

            const Float diffexp = lightSources_[i]->diffuseExponent(thread, time);
            l->setDiffuseExponent(i, diffexp);

            if (mix > 0)
            {
                const Vec4 dir = lightSources_[i]->lightDirection(thread, time);
                l->setDirection(i, dir[0], dir[1], dir[2], dir[3]);
            }
        }
        else
            // XXX there should be a runtime switch in shader!
            l->setColor(i, 0,0,0,1);
    }
}

void Scene::calculateSceneTransform(uint thread, uint sample, Double time)
{
    ScopedSceneLockRead lock(this);
    calculateSceneTransform_(thread, sample, time);
}

void Scene::calculateSceneTransform_(uint thread, uint sample, Double time)
{
    // interpolate free camera
    if (freeCameraIndex_ >= 0)
    {
        freeCameraMatrixGfx_ = glm::inverse(freeCameraMatrix_);
        //freeCameraMatrixGfx_ += (Float)1 / 30 *
        //        (glm::inverse(freeCameraMatrix_) - freeCameraMatrixGfx_);
    }

    // set the initial matrix for all objects in scene
    clearTransformation(thread, sample);

    int camcount = 0;

    // calculate transformations
    for (auto &o : posObjects_)
    {
        // see if this object is a user-controlled camera
        bool freecam = false;
        if (o->isCamera())
        {
            if (camcount == freeCameraIndex_)
                freecam = true;
            ++camcount;
        }

        if (o->active(time, thread))
        {
            if (!freecam)
            {
                // get parent transformation
                Mat4 matrix(o->parentObject()->transformation(thread, sample));
                // apply object's transformation
                o->calculateTransformation(matrix, time, thread);
                // write back
                o->setTransformation(thread, sample, matrix);
            }
            else
                o->setTransformation(thread, sample, freeCameraMatrixGfx_);

            o->updateAudioTransformations(time, thread);
        }
    }
}


// ---------------------- runtime --------------------------

void Scene::lockRead_()
{
    readWriteLock_->lockForRead();
}

void Scene::lockWrite_()
{
    readWriteLock_->lockForWrite();
}

void Scene::unlock_()
{
    readWriteLock_->unlock();
}

void Scene::kill()
{
    if (!glContext_)
        return;

    // release opengl resources later in their thread
    for (uint i=0; i<releaseAllGlRequested_.size(); ++i)
        releaseAllGlRequested_[i] = true;

#if 1
    // kill now (The calling thread must be GUI thread!)
    glContext_->makeCurrent();
    renderScene(0, MO_GFX_THREAD);
    isShutDown_ = true;
#else
    // move to later (Window must repaint!)
    render_();
#endif
}

void Scene::setSceneTime(Double time, bool send_signal)
{
    sceneTime_ = time;
    samplePos_ = time * sampleRate();

    CurrentTime::setTime(time);

    if (send_signal)
        emit sceneTimeChanged(sceneTime_);

//    render_();
}

void Scene::setSceneTime(SamplePos pos, bool send_signal)
{
    sceneTime_ = pos * sampleRateInv();
    samplePos_ = pos;

    CurrentTime::setTime(sceneTime_);

    if (send_signal)
        emit sceneTimeChanged(sceneTime_);
//    render_();
}


void Scene::setProjectionSettings(const ProjectionSystemSettings & p)
{
    *projectionSettings_ = p;

    // update all cameras
    for (Camera * c : cameras_)
        for (uint i=0; i<c->numberThreads(); ++i)
            c->p_needsInitGl_[i] = true;

    render_();
}

void Scene::setProjectorIndex(uint index)
{
    if (index == projectorIndex_)
        return;

    projectorIndex_ = index;

    // update all cameras
    for (Camera * c : cameras_)
        for (uint i=0; i<c->numberThreads(); ++i)
            c->p_needsInitGl_[i] = true;

    render_();
}

} // namespace MO
