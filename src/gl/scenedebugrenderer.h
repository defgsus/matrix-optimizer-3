/** @file scenedebugrenderer.h

    @brief Renderer for "invisible objects" for Scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#ifndef MOSRC_GL_SCENEDEBUGRENDERER_H
#define MOSRC_GL_SCENEDEBUGRENDERER_H

#include <QList>

#include "object/object_fwd.h"
#include "gl/opengl_fwd.h"

namespace MO {
namespace GL {

class SceneDebugRenderer
{
public:
    SceneDebugRenderer(Scene *);
    ~SceneDebugRenderer();

    /** Call when changes to objects has happened */
    void updateTree();

    bool isGlInitialized() const;
    void initGl();
    void releaseGl();

    /** @p options is an OR combination of Scene::DebugRenderOption */
    void render(const RenderSettings&, uint thread, int options = 0xffffffff);

private:

    struct Private;
    Private * p_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCENEDEBUGRENDERER_H
