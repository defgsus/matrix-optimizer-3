/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>
#include <QSemaphore>

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
#include "model/objecttreemodel.h"

namespace MO {

class ScopedSceneLock
{
    Scene * scene_;
    int num_;
public:
    ScopedSceneLock(Scene * scene, int num = -1)
        :   scene_  (scene),
            num_    (num)
    {
        scene_->lock_(num_);
    }

    ~ScopedSceneLock() { scene_->unlock_(num_); }
};


MO_REGISTER_OBJECT(Scene)

Scene::Scene(QObject *parent) :
    Object      (parent),
    model_      (0),
    glContext_  (0),
    numThreads_ (2),
    sceneTime_  (0)
{
    setName("Scene");

    semaphore_ = new QSemaphore(numThreads_);

    timer_.setInterval(1000 / 60);
    timer_.setSingleShot(false);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(timerUpdate_()));
}

Scene::~Scene()
{
    stop();

    if (semaphore_)
        delete semaphore_;
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

    allObjects_ = findChildObjects<Object>(QString(), true);
    allObjects_.prepend(this);
    cameras_ = findChildObjects<Camera>(QString(), true);
    glObjects_ = findChildObjects<ObjectGl>(QString(), true);

#ifdef MO_DO_DEBUG_TREE
    for (auto o : allObjects_)
        MO_DEBUG_TREE("object: " << o << ", parent: " << o->parentObject());
#endif

    // not all objects need there transformation calculated
    // these are the ones that do
    posObjects_ = findChildObjects(TG_REAL_OBJECT, true);

    // get the path until each camera
    cameraPaths_.clear();
    for (auto cam : cameras_)
    {
        cameraPaths_.append(QList<Object*>());
        Object * o = cam;
        while (o && o != this)
        {
            cameraPaths_.back().prepend(o);
            o = o->parentObject();
        }
    }

#if (0)
    MO_DEBUG("Scene: " << cameras_.size() << " cameras, "
             << glObjects_.size() << " gl-objects"
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
    if (!timer_.isActive())
        emit renderRequest();
}



// ----------------------- tree ------------------------------

void Scene::addObject(Object *parent, Object *newChild, int insert_index)
{
    MO_DEBUG_TREE("Scene::addObject(" << parent << ", " << newChild << ", " << insert_index << ")");

    {
        ScopedSceneLock lock(this);
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
        ScopedSceneLock lock(this);
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
        ScopedSceneLock lock(this);
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

    updateChildrenChanged_();

    // tell all objects how much thread data they need
    updateNumberThreads_();

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
    MO_DEBUG_TREE("Scene::updateNumberThreads_() numThreads_ == " << numThreads_);

    for (auto o : allObjects_)
        if (o->numberThreads() != numThreads_)
            o->setNumberThreads(numThreads_);
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


// -------------------- parameter ----------------------------

void Scene::setParameterValue(ParameterFloat *p, Double v)
{
    {
        ScopedSceneLock lock(this);
        p->setValue(v);
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    render_();
}

void Scene::setParameterValue(ParameterSelect *p, int v)
{
    {
        ScopedSceneLock lock(this);
        p->setValue(v);
        p->object()->onParameterChanged(p);
    }
    emit parameterChanged(p);
    render_();
}

void Scene::addModulator(Parameter *p, const QString &idName)
{
    {
        ScopedSceneLock lock(this);
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
        ScopedSceneLock lock(this);
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
        ScopedSceneLock lock(this);
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
    lock_();
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
    lock_();
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
    lock_();
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

void Scene::calculateAudioBlock(SamplePos samplePos, SamplePos blockLength, int thread)
{
    ScopedSceneLock lock(this, 1);

    blockLength += samplePos;
    for (; samplePos<blockLength; ++samplePos)
    {
        calculateSceneTransform_(thread, (1.0 / 44100.0) * samplePos);
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

void Scene::renderScene(Double time)
{
    //qDebug() << "Scene::renderScene("<<time<<")";

    MO_ASSERT(glContext_, "renderScene() without context");

    if (!glContext_ || cameras_.empty())
        return;

    time = sceneTime_;

    {
        ScopedSceneLock lock(this, 1);

        // initialize gl resources
        for (auto o : glObjects_)
            if (o->needsInitGl(0) && o->active(time))
                o->initGl_(0);

        calculateSceneTransform(0, time);

        // start camera frame
        cameras_[0]->startGlFrame(0, time);

        // render all opengl objects
        for (auto o : glObjects_)
        if (o->active(time))
        {
            o->renderGl_(0, time);
        }
    }
}

void Scene::calculateSceneTransform(int thread, Double time)
{
    ScopedSceneLock lock(this, 1);
    calculateSceneTransform_(thread, time);
}

void Scene::calculateSceneTransform_(int thread, Double time)
{
    if (!cameras_.size())
        return;

    // get camera transform
    Mat4 camt(1.0);
    for (auto o : cameraPaths_[0])
        if (o->active(time))
            o->calculateTransformation(camt, time);

    // set the initial camera space for all objects in scene
    setTransformation(thread, glm::inverse(camt));

    // calculate transformations
    for (auto &o : posObjects_)
    if (o->active(time))
    {
        // get parent transformation
        Mat4 matrix(o->parentObject()->transformation(thread));
        // apply object's transformation
        o->calculateTransformation(matrix, time);
        // write back
        o->setTransformation(thread, matrix);
    }

}


// ---------------------- runtime --------------------------

void Scene::lock_(int num)
{
    semaphore_->acquire(num>0? num : numThreads_);
}

void Scene::unlock_(int num)
{
    semaphore_->release(num>0? num : numThreads_);
}

void Scene::setSceneTime(Double time, bool send_signal)
{
    sceneTime_ = time;
    if (send_signal)
        emit sceneTimeChanged(sceneTime_);
    render_();
}


void Scene::start()
{
    timer_.start();
}

void Scene::stop()
{
    if (timer_.isActive())
        timer_.stop();
    else
        setSceneTime(0);
}

void Scene::timerUpdate_()
{
    sceneTime_ += (Double)timer_.interval()/1000;
    emit sceneTimeChanged(sceneTime_);
    emit renderRequest();
}

} // namespace MO
