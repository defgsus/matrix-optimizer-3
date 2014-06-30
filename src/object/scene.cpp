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

namespace MO {

MO_REGISTER_OBJECT(Scene)

Scene::Scene(QObject *parent) :
    Object      (parent),
    glContext_  (0)
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


// ----------------------- open gl ---------------------------

void Scene::setGlContext(GL::Context *context)
{
    MO_DEBUG_GL("Scene::setGlContext(" << context << ")");

    glContext_ = context;

    MO_DEBUG_GL("setting gl context for objects");
    for (auto o : glObjects_)
        o->setGlContext_(glContext_);

}

void Scene::renderScene(Double time)
{
    MO_ASSERT(glContext_, "renderScene() without context");

    if (!glContext_ || cameras_.empty())
        return;

    // initialize gl resources
    for (auto o : glObjects_)
        if (o->needsInitGl())
            o->initGl_();

    // calculate transformations
    for (auto &o : posObjects_)
    {
        o.matrix = o.object->parentObject()->transformation();
        o.object->calculateTransformation(o.matrix, time);
        o.object->setTransformation(o.matrix);
    }

    // start camera frame
    cameras_[0]->startGlFrame(time);

    // render all opengl objects
    for (auto o : glObjects_)
    {
        o->renderGl_(time);
    }
}


} // namespace MO
