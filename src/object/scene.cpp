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
    cameras_ = findChildObjects<Camera>();
    glObjects_ = findChildObjects<ObjectGl>();

    MO_DEBUG("Scene: " << cameras_.size() << " cameras, "
             << glObjects_.size() << " gl-objects"
             );
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

    if (!glContext_)
        return;

    for (auto o : glObjects_)
    {
        if (o->needsInitGl())
            o->initGl_();

        o->renderGl_(time);
    }
}


} // namespace MO
