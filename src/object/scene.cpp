/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

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
    numThreads_ (1)
{
    setName("Scene");
}



void Scene::treeChanged()
{
    findObjects_();

    if (glContext_)
        setGlContext(glContext_);
}

void Scene::findObjects_()
{
    //MO_DEBUG("Scene::findObjects_()");

    allObjects_ = findChildObjects<Object>(QString(), true);
    cameras_ = findChildObjects<Camera>(QString(), true);
    glObjects_ = findChildObjects<ObjectGl>(QString(), true);

    auto objs = findChildObjects(T_REAL_OBJECTS, true);
    posObjects_.clear();
    for (auto o : objs)
        posObjects_.append(PositionalObject_(o));

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

// -------------------- parameter ----------------------------

void Scene::setParameterValue(ParameterFloat *p, Double v)
{
    p->setValue(v);
    emit renderRequest();
}

// ----------------------- open gl ---------------------------

void Scene::setGlContext(GL::Context *context)
{
    MO_DEBUG_GL("Scene::setGlContext(" << context << ")");

    glContext_ = context;

    updateNumberThreads_();

    MO_DEBUG_GL("setting gl context for objects");
    for (auto o : glObjects_)
        o->setGlContext_(0, glContext_);

}

void Scene::renderScene(Double time)
{
    MO_ASSERT(glContext_, "renderScene() without context");

    if (!glContext_ || cameras_.empty())
        return;

    updateNumberThreads_();

    // initialize gl resources
    for (auto o : glObjects_)
        if (o->needsInitGl(0))
            o->initGl_(0);

    // apply camera transform
    Mat4 camt(1.0);
    cameras_[0]->calculateTransformation(camt, time);
    setTransformation(0, glm::inverse(camt));

    // calculate transformations
    for (auto &o : posObjects_)
    {
        o.matrix = o.object->parentObject()->transformation(0);
        o.object->calculateTransformation(o.matrix, time);
        o.object->setTransformation(0, o.matrix);
    }

    // start camera frame
    cameras_[0]->startGlFrame(0, time);

    // render all opengl objects
    for (auto o : glObjects_)
    {
        o->renderGl_(0, time);
    }
}


} // namespace MO
