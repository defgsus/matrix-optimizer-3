/** @file scene.cpp

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/28/2014</p>
*/

#include "scene.h"

#include "camera.h"

namespace MO {


Scene::Scene(QObject *parent) :
    Object(parent)
{
}




void Scene::findObjects_()
{
    cameras_ = findChildObjects<Camera>();
    glObjects_ = findChildObjects<ObjectGl>();
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
    glContext_ = context;

    for (auto o : glObjects_)
        o->setGlContext(glContext_);
}



} // namespace MO
