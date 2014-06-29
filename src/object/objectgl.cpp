/** @file objectgl.cpp

    @brief Abstract openGL object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#include "objectgl.h"
#include "io/error.h"
#include "gl/context.h"

namespace MO {


ObjectGl::ObjectGl(QObject *parent)
    :   Object3d    (parent),
        glContext_  (0),
        glFunctionsInitialized_(false)
{
}


void ObjectGl::setGlContext_(GL::Context * c)
{
    if (c != glContext_)
        glFunctionsInitialized_ = false;

    glContext_ = c;
}

void ObjectGl::render_()
{
    if (!glContext_)
        MO_GL_ERROR("no context defined for object '" << idName() << "'");
    if (!glContext_->isValid())
        MO_GL_ERROR("context not initialized for object '" << idName() << "'");

    if (!glFunctionsInitialized_)
    {
        bool r = initializeOpenGLFunctions();
        if (!r)
            MO_GL_ERROR("could not initialize opengl functions for object '" << idName() << "'");
    }

    render();
}


} // namespace MO
