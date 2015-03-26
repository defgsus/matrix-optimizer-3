/** @file offscreencontext.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.03.2015</p>
*/

#include <QOffscreenSurface>

#include "offscreencontext.h"
#include "scenerenderer.h"
#include "gl/opengl.h"

#include "gl/opengl_undef.h"
#include <QOpenGLContext>

namespace MO {
namespace GL {



OffscreenContext::OffscreenContext(QObject *parent)
    : Context       (parent)
    , p_os_surface_ (0)
{
}

OffscreenContext::~OffscreenContext()
{
    if (p_os_surface_)
    {
        if (p_os_surface_->isValid())
            p_os_surface_->destroy();

        delete p_os_surface_;
    }
}

bool OffscreenContext::createGl()
{
    if (p_os_surface_ && p_os_surface_->isValid())
        p_os_surface_->destroy();

    if (!p_os_surface_)
        p_os_surface_ = new QOffscreenSurface();

    // create QOffscrenSurface
    p_os_surface_->setFormat(SceneRenderer::defaultFormat());
    p_os_surface_->create();
    if (!p_os_surface_->isValid())
        return false;

    setSurface(p_os_surface_);
    qcontext()->setFormat(p_os_surface_->format());

    if (!qcontext()->create())
        return false;

    glbinding::Binding::initialize();

    return true;
}

bool OffscreenContext::destroyGl()
{
    if (!p_os_surface_)
        return false;

    p_os_surface_->destroy();

    return true;
}

} // namespace GL
} // namespace MO
