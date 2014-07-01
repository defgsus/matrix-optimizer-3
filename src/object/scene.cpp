/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

//#include <QDebug>

#include "scene.h"

#include "camera.h"
#include "io/error.h"
#include "io/log.h"
#include "object/parameterfloat.h"

namespace MO {

MO_REGISTER_OBJECT(Scene)

Scene::Scene(QObject *parent) :
    Object      (parent),
    glContext_  (0),
    numThreads_ (1),
    sceneTime_  (0)
{
    setName("Scene");

    timer_.setInterval(1000 / 30);
    timer_.setSingleShot(false);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(timerUpdate_()));
}



void Scene::treeChanged()
{
    MO_DEBUG_TREE("Scene::treeChanged()");

    findObjects_();

    if (glContext_)
    {
        // update infos for new objects
        setGlContext(glContext_);

        // update image
        emit renderRequest();
    }
}

void Scene::findObjects_()
{
    //MO_DEBUG_TREE("Scene::findObjects_()");

    allObjects_ = findChildObjects<Object>(QString(), true);
    cameras_ = findChildObjects<Camera>(QString(), true);
    glObjects_ = findChildObjects<ObjectGl>(QString(), true);

    // not all objects need there transformation calculated
    // these are the ones that do
    posObjects_ = findChildObjects(TG_REAL_OBJECT, true);

    // tell all object how much thread data they need
    updateNumberThreads_();

    // collect all modulators for each object
    updateModulators_();

#if (0)
    MO_DEBUG("Scene: " << cameras_.size() << " cameras, "
             << glObjects_.size() << " gl-objects"
             );
#endif
}

void Scene::initGlChilds_()
{
    /*for (auto o : glObjects_)
    {

    }*/
}

void Scene::updateNumberThreads_()
{
    //MO_DEBUG("Scene::updateNumberThreads_() ");

    // update scene as well!
    if (numberThreads() != numThreads_)
        setNumberThreads(numThreads_);

    // update all objects
    for (auto o : allObjects_)
        if (o->numberThreads() != numThreads_)
            o->setNumberThreads(numThreads_);
}

void Scene::updateModulators_()
{
    // update scene as well!
    collectModulators();

    // update all objects
    for (auto o : allObjects_)
        o->collectModulators();
}

// -------------------- parameter ----------------------------

void Scene::setParameterValue(ParameterFloat *p, Double v)
{
    p->setValue(v);
    emit renderRequest();
}

// --------------------- sequence ----------------------------

void Scene::beginSequenceChange(Sequence * s)
{
    MO_DEBUG_PARAM("Scene::beginSequenceChange(" << s << ")");
    s = s;
}

void Scene::endSequenceChange()
{
    MO_DEBUG_PARAM("Scene::endSequenceChange()");
}

// --------------------- objects -----------------------------

void Scene::beginObjectChange(Object * o)
{
    MO_DEBUG_PARAM("Scene::beginObjectChange(" << o << ")");
    o = o;
}

void Scene::endObjectChange()
{
    MO_DEBUG_PARAM("Scene::endObjectChange()");
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

    // initialize gl resources
    for (auto o : glObjects_)
        if (o->needsInitGl(0))
            o->initGl_(0);

    calculateSceneTransform(0, time);

    // start camera frame
    cameras_[0]->startGlFrame(0, time);

    // render all opengl objects
    for (auto o : glObjects_)
    {
        o->renderGl_(0, time);
    }
}

void Scene::calculateSceneTransform(int thread, Double time)
{
    if (!cameras_.size())
        return;

    // apply camera transform
    Mat4 camt(1.0);
    cameras_[thread]->calculateTransformation(camt, time);
    setTransformation(thread, glm::inverse(camt));

    // calculate transformations
    for (auto &o : posObjects_)
    {
        // get parent transformation
        Mat4 matrix(o->parentObject()->transformation(thread));
        // apply object's transformation
        o->calculateTransformation(matrix, time);
        // write back
        o->setTransformation(thread, matrix);
    }

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
    emit renderRequest();
}

} // namespace MO
