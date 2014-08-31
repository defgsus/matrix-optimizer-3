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

    /** Call when changes to objects has happened */
    void updateTree();

    void initGl();
    void releaseGl();

    void render(const RenderSettings&, uint thread);

private:

    Scene * scene_;
    QList<Camera*> cameras_;
    QList<Microphone*> microphones_;
    QList<AUDIO::AudioSource*> audioSources_;

    GL::Drawable
        * drawCamera_,
        * drawSoundSource_,
        * drawMicrophone_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SCENEDEBUGRENDERER_H
