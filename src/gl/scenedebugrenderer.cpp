/** @file scenedebugrenderer.cpp

    @brief Renderer for "invisible objects" for Scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include "scenedebugrenderer.h"
#include "object/scene.h"
#include "object/soundsource.h"
#include "object/camera.h"
#include "object/microphone.h"
#include "gl/drawable.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "geom/geometry.h"
#include "geom/objloader.h"

namespace MO {
namespace GL {


SceneDebugRenderer::SceneDebugRenderer(Scene * s)
    : scene_        (s)
{
}

void SceneDebugRenderer::updateTree()
{
    cameras_ = scene_->findChildObjects<Camera>(QString(), true);
    microphones_ = scene_->findChildObjects<Microphone>(QString(), true);

    QList<Object*> all = scene_->findChildObjects(Object::TG_ALL, true);
    audioSources_.clear();
    for (Object * o : all)
        audioSources_.append( o->audioSources() );
}

void SceneDebugRenderer::initGl()
{
    GEOM::ObjLoader objload;

    // default wire-frame shader
    GL::ShaderSource src;
    src.loadDefaultSource();

    // --- setup camera drawable ----

    drawCamera_ = new GL::Drawable("scene_debug_camera");
    objload.loadFile(":/model/camera.obj");
    objload.getGeometry(drawCamera_->geometry());
    drawCamera_->setShaderSource(src);
    drawCamera_->createOpenGl();
}

void SceneDebugRenderer::releaseGl()
{
    if (drawCamera_->isReady())
        drawCamera_->releaseOpenGl();
    delete drawCamera_;
}

void SceneDebugRenderer::render(const RenderSettings & rs, uint thread)
{
    const Mat4&
            proj = rs.cameraSpace().projectionMatrix(),
            cubeView = rs.cameraSpace().cubeViewMatrix(),
            view = rs.cameraSpace().viewMatrix();

    for (Camera * o : cameras_)
    {
        const Mat4& trans = o->transformation(thread, 0);
        drawCamera_->renderShader(proj, cubeView * trans, view * trans, trans);
    }
}



} // namespace GL
} // namespace MO
