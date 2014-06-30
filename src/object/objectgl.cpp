/** @file objectgl.cpp

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

//#include <QDebug>

#include "objectgl.h"
#include "io/error.h"
#include "gl/context.h"

namespace MO {


ObjectGl::ObjectGl(QObject *parent)
    :   Object      (parent),
        glFunctionsInitialized_(false)
{
}

void ObjectGl::setNumberThreads(int num)
{
    Object::setNumberThreads(num);

    glContext_.resize(num);
    needsInitGl_.resize(num);
}

void ObjectGl::setGlContext_(int thread, GL::Context * c)
{
    if (c != glContext_[thread])
    {
        //glFunctionsInitialized_ = false;
        needsInitGl_[thread] = true;
    }

    glContext_[thread] = c;
}

void ObjectGl::initGl_(int thread)
{
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
}

void ObjectGl::renderGl_(int thread, Double time)
{
    if (!glContext_[thread])
        MO_GL_ERROR("no context["<<thread<<"] defined for object '" << idName() << "'");
    if (!glContext_[thread]->isValid())
        MO_GL_ERROR("context["<<thread<<"] not initialized for object '" << idName() << "'");

    if (!glFunctionsInitialized_)
        MO_GL_ERROR("opengl functions not initialized for object '" << idName() << "'");

    renderGl(thread, time);
}


} // namespace MO
