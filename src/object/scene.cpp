/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>
#include <QReadWriteLock>

#include "scene.h"

#include "camera.h"
#include "io/error.h"
#include "io/log.h"
#include "io/datastream.h"
#include "object/objectfactory.h"
#include "object/param/parameterfloat.h"
#include "object/param/parameterselect.h"
#include "object/track.h"
#include "object/sequencefloat.h"
#include "object/microphone.h"
#include "model/objecttreemodel.h"
#include "audio/audiodevice.h"
#include "audio/audiosource.h"
#include "gl/cameraspace.h"
#include "gl/framebufferobject.h"
#include "gl/screenquad.h"
#include "gl/texture.h"

namespace MO {

class ScopedSceneLockRead
{
    Scene * scene_;
public:
    ScopedSceneLockRead(Scene * scene) : scene_(scene)
    {
        scene_->lockRead_();
    }

    ~ScopedSceneLockRead() { scene_->unlock_(); }
};

class ScopedSceneLockWrite
{
    Scene * scene_;
public:
    ScopedSceneLockWrite(Scene * scene) : scene_(scene)
    {
        scene_->lockWrite_();
    }

    ~ScopedSceneLockWrite() { scene_->unlock_(); }
};


MO_REGISTER_OBJECT(Scene)

Scene::Scene(QObject *parent) :
    Object              (parent),
    model_              (0),
    glContext_          (0),
    fbWidth_            (512),
    fbHeight_           (512),
    fbFormat_           (GL_RGBA),
    fboFinal_           (0),
    sceneNumberThreads_ (2),
    sceneSampleRate_    (44100),
    audioDevice_        (new AUDIO::AudioDevice()),
    isPlayback_         (false),
    sceneTime_          (0),
    samplePos_          (0)
{
    MO_DEBUG("Scene::Scene()");

    setName("Scene");

    readWriteLock_ = new QReadWriteLock();
/*
    timer_.setInterval(1000 / 60);
    timer_.setSingleShot(false);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(timerUpdate_()));
*/
    sceneBufferSize_.resize(sceneNumberThreads_);
    sceneBufferSize_[0] = 1;
    sceneBufferSize_[1] = 32;

    sceneDelaySize_.resize(sceneNumberThreads_);
    sceneDelaySize_[0] = 0;
    sceneDelaySize_[1] = 1<<16;
}

Scene::~Scene()
{
    MO_DEBUG("Scene::~Scene()");

    stop();
    delete audioDevice_;
    delete readWriteLock_;
}

void Scene::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("scene", 1);
}

void Scene::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("scene", 1);
}

void Scene::setObjectModel(ObjectTreeModel * model)
{
    model_ = model;
    model_->setSceneObject(this);
}


void Scene::findObjects_()
{
    MO_DEBUG_TREE("Scene::findObjects_()");

    // all objects including scene
    allObjects_ = findChildObjects<Object>(QString(), true);
    allObjects_.prepend(this);
    // all cameras
    cameras_ = findChildObjects<Camera>(QString(), true);
    // all objects that need to be rendered
    glObjects_ = findChildObjects<ObjectGl>(QString(), true);
    // not all objects need there transformation calculated
    // these are the ones that do
    posObjects_ = findChildObjects(TG_REAL_OBJECT, true);
    // all objects with audio sources
    audioObjects_.clear();
    for (auto o : allObjects_)
        if (!o->audioSources().isEmpty())
            audioObjects_.append(o);
    // all microphones
    microphones_ = findChildObjects<Microphone>(QString(), true);

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
    if (!isPlayback())
        emit renderRequest();
}



// ----------------------- tree ------------------------------

void Scene::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_TREE("Scene::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");

    {
        ScopedSceneLockWrite lock(this);
        parent->addObject_(newChild, insert_index);
        parent->childrenChanged_();
        updateTree_();
    }
    emit objectAdded(newChild);
    render_();
}

void Scene::deleteObject(Object *object)
{
    MO_DEBUG_TREE("Scene::deleteObject(" << object << ")");

    MO_ASSERT(object->parentObject(), "Scene::deleteObject("<<object<<") without parent");
    {
        ScopedSceneLockWrite lock(this);
        Object * parent = object->parentObject();
        parent->deleteObject_(object);
        parent->childrenChanged_();
        updateTree_();
    }
    emit objectDeleted(object);
    render_();
}

void Scene::swapChildren(Object *parent, int from, int to)
{
    MO_DEBUG_TREE("Scene::swapChildren(" << parent << ", " << from << ", " << to << ")");

    {
        ScopedSceneLockWrite lock(this);
        parent->swapChildren_(from, to);
        parent->childrenChanged_();
        updateTree_();
    }
    emit childrenSwapped(parent, from, to);
    render_();
}


void Scene::updateTree_()
{
    MO_DEBUG_TREE("Scene::updateTree_()");

    findObjects_();

    // tell all objects if there children have changed
    updateChildrenChanged_();

    // tell all objects how much thread data they need
    updateNumberThreads_();
    updateBufferSize_();
    updateSampleRate_();
    updateDelaySize_();

    // get buffers for microphones
    updateAudioBuffers_();

    // collect all modulators for each object
    updateModulators_();

    if (glContext_)
    {
        // update infos for new objects
        setGlContext(glContext_);

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

    if (numberThreads() != sceneNumberThreads_)
        setNumberThreads(sceneNumberThreads_);

    for (auto o : allObjects_)
        if (o->numberThreads() != sceneNumberThreads_)
            o->setNumberThreads(sceneNumberThreads_);
}


void Scene::updateBufferSize_()
{
    MO_DEBUG_AUDIO("Scene::updateBufferSize_() numberThreads() == " << numberThreads());

    for (uint i=0; i<sceneNumberThreads_; ++i)
        if (bufferSize(i) != sceneBufferSize_[i])
            setBufferSize(sceneBufferSize_[i], i);

#ifdef MO_DO_DEBUG_AUDIO
    for (uint i=0; i<sceneNumberThreads_; ++i)
        MO_DEBUG_AUDIO("bufferSize("<<i<<") == " << bufferSize(i));
#endif

    for (auto o : allObjects_)
    {
        for (uint i=0; i<sceneNumberThreads_; ++i)
        {
            if (o->bufferSize(i) != sceneBufferSize_[i])
                o->setBufferSize(sceneBufferSize_[i], i);
        }
    }
}

void Scene::updateDelaySize_()
{
    for (auto o : audioObjects_)
        for (auto a : o->audioSources())
            for (uint i=0; i<numberThreads(); ++i)
                a->setDelaySize(sceneDelaySize_[i], i);
}

void Scene::updateAudioBuffers_()
{
    MO_DEBUG_AUDIO("Scene::updateAudioBuffers_() numberThreads() == " << numberThreads());

    audioOutput_.resize(numberThreads());

    for (uint i=0; i<numberThreads(); ++i)
    {
        audioOutput_[i].resize(bufferSize(i) * microphones_.size());

        memset(&audioOutput_[i][0], 0, sizeof(F32) * bufferSize(i) * microphones_.size());

        MO_DEBUG_AUDIO("audioOutput_[" << i << "].size() == "
                       << audioOutput_[i].size());
    }
}

void Scene::updateModulators_()
{
    MO_DEBUG_TREE("Scene::updateModulators_()");

    for (auto o : allObjects_)
    {
        o->collectModulators();
        // check parameters as well
        for (auto p : o->parameters())
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

void Scene::setParameterValue(ParameterFloat *p, Double v)
{
    {
        ScopedSceneLockWrite lock(this);
        p->setValue(v);
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    if (Sequence * seq = qobject_cast<Sequence*>(p->object()))
        emit sequenceChanged(seq);
    render_();
}

void Scene::setParameterValue(ParameterSelect *p, int v)
{
    {
        ScopedSceneLockWrite lock(this);
        p->setValue(v);
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    if (Sequence * seq = qobject_cast<Sequence*>(p->object()))
        emit sequenceChanged(seq);
    render_();
}

void Scene::addModulator(Parameter *p, const QString &idName)
{
    {
        ScopedSceneLockWrite lock(this);
        p->addModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    render_();
}

void Scene::removeModulator(Parameter *p, const QString &idName)
{
    {
        ScopedSceneLockWrite lock(this);
        p->removeModulator(idName);
        p->collectModulators();
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    render_();
}

void Scene::removeAllModulators(Parameter *p)
{
    {
        ScopedSceneLockWrite lock(this);
        p->removeAllModulators();
        p->collectModulators();
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    render_();
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
    changedSequence_ = s;
}

void Scene::endSequenceChange()
{
    MO_DEBUG_PARAM("Scene::endSequenceChange()");
    unlock_();
    emit sequenceChanged(changedSequence_);
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
        emit sequenceChanged(s);

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
    emit objectChanged(changedObject_);
    render_();
}



// ------------------------ audio ----------------------------

void Scene::calculateAudioBlock(SamplePos samplePos, uint thread)
{
    ScopedSceneLockRead lock(this);

    const uint size = bufferSize(thread);

    const Double time = sampleRateInv() * samplePos;
    Double rtime = time;

    // calculate one block of transformations
    // XXX needs only be done for audioobjects and microphones
    for (uint i = 0; i<size; ++i)
    {
        calculateSceneTransform_(thread, i, rtime);
        rtime += sampleRateInv();
    }

    // calculate audio objects
    for (auto o : audioObjects_)
    {
        o->performAudioBlock(samplePos, thread);
        // fill delay lines
        for (auto a : o->audioSources())
            a->pushDelay(thread);
    }

    for (int i = 0; i<microphones_.size(); ++i)
    {
        auto mic = microphones_[i];

        // clear audio buffer
        F32 * buffer = &audioOutput_[thread][i * size];
        memset(buffer, 0, sizeof(F32) * size);

        // for each object
        for (auto o : audioObjects_)
            // for each source in object
            for (auto src : o->audioSources())
                // add to microphone 'membrane'
                mic->sampleAudioSource(src, buffer, thread);
    }
}

void Scene::getAudioOutput(uint numChannels, uint thread, F32 *buffer) const
{
    const uint size = bufferSize(thread);

    // clear buffer
    memset(buffer, 0, sizeof(F32) * size * numChannels);

    // rearange the audioOutput buffer

    const F32* src = &audioOutput_[thread][0];

    const uint chan = std::min(numChannels, (uint)microphones_.size());
    for (uint b = 0; b < size; ++b)
    {
        for (uint c = 0; c < chan; ++c)
        {
            *buffer++ = src[c * size + b];
        }

        // skip unused channels
        buffer += (numChannels - chan);
    }
}

// ----------------------- open gl ---------------------------

void Scene::setGlContext(GL::Context *context)
{
    MO_DEBUG_GL("Scene::setGlContext(" << context << ")");

    glContext_ = context;

    MO_DEBUG_GL("setting gl context for objects");
    for (auto o : glObjects_)
        o->setGlContext_(0, glContext_);
}

void Scene::createGl_()
{
    MO_DEBUG_GL("Scene::createGl_()");

    fboFinal_ = new GL::FrameBufferObject(
                fbWidth_, fbHeight_, fbFormat_, GL_FLOAT, false, GL::ER_THROW);
    fboFinal_->create();
    fboFinal_->unbind();

    // create screen quad
    screenQuad_ = new GL::ScreenQuad("scene_quad", GL::ER_THROW);
    screenQuad_->create();
}

void Scene::renderScene(Double time, uint thread)
{
    //MO_DEBUG_GL("Scene::renderScene("<<time<<", "<<thread<<")");

    MO_ASSERT(glContext_, "renderScene() without context");

    if (!glContext_ || cameras_.empty())
        return;

    time = sceneTime_;

    {
        // read-lock is sufficient because we
        // modify only thread-local storage
        ScopedSceneLockRead lock(this);

        if (!fboFinal_)
            createGl_();

        // initialize gl resources
        for (auto o : glObjects_)
            if (o->needsInitGl(thread) && o->active(time))
                o->initGl_(thread);

        calculateSceneTransform(thread, 0, time);

        for (auto camera : cameras_)
        if (camera->active(time))
        {
            // start camera frame
            camera->startGlFrame(thread, time);

            GL::CameraSpace camSpace;
            camera->initCameraSpace(camSpace, thread, 0);
            camSpace.setViewMatrix( glm::inverse(camera->transformation(thread, 0)) );

            // render all opengl objects
            for (auto o : glObjects_)
            if (o->active(time))
            {
                o->renderGl_(camSpace, thread, time);
            }

            camera->finishGlFrame(thread, time);
        }
    }

    // mix camera frames
    fboFinal_->bind();
    fboFinal_->setViewport();
    MO_CHECK_GL( glClearColor(0, 0, 0, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    MO_CHECK_GL( glDisable(GL_DEPTH_TEST) );
    for (auto camera : cameras_)
        if (camera->active(time))
            camera->drawFramebuffer(thread, time);
    fboFinal_->unbind();

    // draw to screen
    fboFinal_->colorTexture()->bind();
    MO_CHECK_GL( glViewport(0, 0, glContext_->size().width(), glContext_->size().height()) );
    MO_CHECK_GL( glClearColor(0.1, 0.1, 0.1, 1.0) );
    MO_CHECK_GL( glClear(GL_COLOR_BUFFER_BIT) );
    screenQuad_->draw(glContext_->size().width(), glContext_->size().height());
    fboFinal_->colorTexture()->unbind();

}

void Scene::calculateSceneTransform(uint thread, uint sample, Double time)
{
    ScopedSceneLockRead lock(this);
    calculateSceneTransform_(thread, sample, time);
}

void Scene::calculateSceneTransform_(uint thread, uint sample, Double time)
{
    // set the initial matrix for all objects in scene
    clearTransformation(thread, sample);

    // calculate transformations
    for (auto &o : posObjects_)
    if (o->active(time))
    {
        // get parent transformation
        Mat4 matrix(o->parentObject()->transformation(thread, sample));
        // apply object's transformation
        o->calculateTransformation(matrix, time);
        // write back
        o->setTransformation(thread, sample, matrix);
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

void Scene::setSceneTime(Double time, bool send_signal)
{
    sceneTime_ = time;
    samplePos_ = time * sampleRate();
    if (send_signal)
        emit sceneTimeChanged(sceneTime_);
    render_();
}

void Scene::setSceneTime(SamplePos pos, bool send_signal)
{
    sceneTime_ = pos * sampleRateInv();
    samplePos_ = pos;
    if (send_signal)
        emit sceneTimeChanged(sceneTime_);
    render_();
}

bool Scene::isAudioInitialized() const
{
    return audioDevice_->ok();
}

void Scene::initAudioDevice_()
{
    if (audioDevice_->isAudioConfigured())
    {
        if (!audioDevice_->initFromSettings())
            return;

        sceneSampleRate_ = audioDevice_->sampleRate();
        sceneBufferSize_[1] = audioDevice_->bufferSize();

        updateBufferSize_();
        updateSampleRate_();
        updateAudioBuffers_();

        using namespace std::placeholders;
        audioDevice_->setCallback(std::bind(
            &Scene::audioCallback_, this, _1, _2));
    }
}

void Scene::closeAudio()
{
    if (isPlayback())
        stop();

    if (isAudioInitialized())
        audioDevice_->close();
}

void Scene::audioCallback_(const F32 *, F32 * out)
{
    MO_ASSERT(audioDevice_->bufferSize() == bufferSize(1),
              "buffer-size mismatch");

    calculateAudioBlock(samplePos_, 1);
    getAudioOutput(audioDevice_->numOutputChannels(), 1, out);

    // update scene time
    setSceneTime(samplePos_ + bufferSize(1));
}

void Scene::start()
{
    ScopedSceneLockWrite lock(this);

    if (!isAudioInitialized())
        initAudioDevice_();

    if (isAudioInitialized())
    {
        isPlayback_ = true;
        audioDevice_->start();

        emit playbackStarted();
    }
}

void Scene::stop()
{
    if (isPlayback())
    {
        isPlayback_ = false;
        if (isAudioInitialized())
            audioDevice_->stop();

        emit playbackStopped();
    }
    else
    {
        setSceneTime(0.0);
    }
    /*
    if (timer_.isActive())
        timer_.stop();
    else
        setSceneTime(0);
    */
}

void Scene::timerUpdate_()
{
    /*
    sceneTime_ += (Double)timer_.interval()/1000;
    emit sceneTimeChanged(sceneTime_);
    emit renderRequest();
    */
}

} // namespace MO
