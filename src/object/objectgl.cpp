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


namespace MO {


ObjectGl::ObjectGl(QObject *parent)
    :   Object      (parent),
        glFunctionsInitialized_(false)
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

    glContext_.resize(num);
    needsInitGl_.resize(num);
}

void ObjectGl::setGlContext_(uint thread, GL::Context * c)
{
    MO_ASSERT(thread < glContext_.size(),
              "setGlContext_(" << thread << ", " << c << ") but "
              "glContext_.size() == " << glContext_.size());

    if (c != glContext_[thread])
    {
        //glFunctionsInitialized_ = false;
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

    if (!glFunctionsInitialized_)
    {
        bool r = initializeOpenGLFunctions();
        if (!r)
            MO_GL_ERROR("could not initialize opengl functions for object '" << idName() << "'");
        glFunctionsInitialized_ = true;
    }

    initGl(thread);

    needsInitGl_[thread] = false;
}

void ObjectGl::renderGl_(const GL::CameraSpace &camera, uint thread, Double time)
{
    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    if (!glFunctionsInitialized_)
        MO_GL_ERROR("opengl functions not initialized for object '" << idName() << "'");

    renderGl(camera, thread, time);
}


} // namespace MO
