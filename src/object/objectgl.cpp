/** @file objectgl.cpp

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

//#include <QDebug>

#include "objectgl.h"
#include "io/error.h"
#include "io/log.h"
#include "gl/context.h"
#include "io/datastream.h"
#include "scene.h"

namespace MO {


ObjectGl::ObjectGl(QObject *parent)
    :   Object      (parent)
{
}

void ObjectGl::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("ogl", 1);
}

void ObjectGl::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("ogl", 1);
}


void ObjectGl::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("ObjectGl::setNumberThreads(" << num << ")");

    Object::setNumberThreads(num);

    uint oldnum = glContext_.size();
    glContext_.resize(num);
    needsInitGl_.resize(num);
    isGlInitialized_.resize(num);

    for (uint i=oldnum; i<num; ++i)
    {
        glContext_[i] = 0;
        needsInitGl_[i] = true;
        isGlInitialized_[i] = false;
    }
}

void ObjectGl::setGlContext_(uint thread, GL::Context * c)
{
    MO_ASSERT(thread < glContext_.size(),
              "setGlContext_(" << thread << ", " << c << ") but "
              "glContext_.size() == " << glContext_.size());

    if (c != glContext_[thread])
    {
        needsInitGl_[thread] = true;
    }

    glContext_[thread] = c;
}

void ObjectGl::initGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::initGl_(" << thread << ")");

    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    initGl(thread);

    needsInitGl_[thread] = false;
    isGlInitialized_[thread] = true;
}


void ObjectGl::releaseGl_(uint thread)
{
    MO_DEBUG_GL("ObjectGl('" << idName() << "')::releaseGl_(" << thread << ")");

    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");

    releaseGl(thread);

    isGlInitialized_[thread] = false;
}

void ObjectGl::renderGl_(const GL::CameraSpace &camera, uint thread, Double time)
{
    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    renderGl(camera, thread, time);
}

void ObjectGl::requestRender()
{
    Scene * scene = sceneObject();
    if (!scene)
        return;

    scene->render();
}

void ObjectGl::requestReinitGl()
{
    for (uint i=0; i<numberThreads(); ++i)
    {
        needsInitGl_[i] = true;
    }

    requestRender();
}

} // namespace MO
