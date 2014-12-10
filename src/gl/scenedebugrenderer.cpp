/** @file scenedebugrenderer.cpp

    @brief Renderer for "invisible objects" for Scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include "scenedebugrenderer.h"
#include "io/log.h"
#include "object/scene.h"
#include "object/soundsource.h"
#include "object/camera.h"
#include "object/microphone.h"
#include "object/lightsource.h"
#include "gl/drawable.h"
#include "gl/shadersource.h"
#include "gl/rendersettings.h"
#include "gl/cameraspace.h"
#include "geom/geometry.h"
#include "geom/objloader.h"
#include "audio/audiosource.h"
#include "audio/audiomicrophone.h"

namespace MO {
namespace GL {


SceneDebugRenderer::SceneDebugRenderer(Scene * s)
    : scene_            (s),
      glReady_          (false),
      drawCamera_       (0),
      drawAudioSource_  (0),
      drawMicrophone_   (0),
      drawLightSource_  (0)
{
}

SceneDebugRenderer::~SceneDebugRenderer()
{
    // only for debugging mainly
    // these objects should be deleted by releaseGl()
    delete drawAudioSource_;
    delete drawCamera_;
    delete drawMicrophone_;
    delete drawLightSource_;
}

void SceneDebugRenderer::updateTree()
{
    cameras_ = scene_->findChildObjects<Camera>(QString(), true);
    lightSources_ = scene_->findChildObjects<LightSource>(QString(), true);

//    QList<Object*> all = scene_->findChildObjects(Object::TG_ALL, true);
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
    drawCamera_->geometry()->convertToLines();
    drawCamera_->geometry()->scale(0.1, 0.1, 0.1);
    addCoordinates_(drawCamera_->geometry());
    drawCamera_->setShaderSource(src);
    drawCamera_->createOpenGl();

    // --- setup AudioSource drawable ----

    drawAudioSource_ = new GL::Drawable("scene_debug_audiosource");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawAudioSource_->geometry());
    drawAudioSource_->geometry()->convertToLines();
    drawAudioSource_->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates_(drawAudioSource_->geometry());
    drawAudioSource_->setShaderSource(src);
    drawAudioSource_->createOpenGl();

    // --- setup LightSource drawable ----

    drawLightSource_ = new GL::Drawable("scene_debug_lightsource");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawLightSource_->geometry());
    drawLightSource_->geometry()->convertToLines();
    drawLightSource_->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates_(drawLightSource_->geometry());
    drawLightSource_->setShaderSource(src);
    drawLightSource_->createOpenGl();

    // --- setup Microphone drawable ----

    drawMicrophone_ = new GL::Drawable("scene_debug_microphone");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawMicrophone_->geometry());
    drawMicrophone_->geometry()->convertToLines();
    drawMicrophone_->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates_(drawMicrophone_->geometry());
    drawMicrophone_->setShaderSource(src);
    drawMicrophone_->createOpenGl();

    glReady_ = true;
}

void SceneDebugRenderer::addCoordinates_(GEOM::Geometry * geom)
{
    const Float len = 1.0;
    geom->setColor(1,0,0,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(len,0,0));
    geom->setColor(0,1,0,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(0,len,0));
    geom->setColor(0,0,1,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(0,0,len));

}

void SceneDebugRenderer::releaseGl()
{
    glReady_ = false;

    if (drawCamera_)
    {
        if (drawCamera_->isReady())
            drawCamera_->releaseOpenGl();
        delete drawCamera_;
        drawCamera_ = 0;
    }

    if (drawMicrophone_)
    {
        if (drawMicrophone_->isReady())
            drawMicrophone_->releaseOpenGl();
        delete drawMicrophone_;
        drawMicrophone_ = 0;
    }

    if (drawAudioSource_)
    {
        if (drawAudioSource_->isReady())
            drawAudioSource_->releaseOpenGl();
        delete drawAudioSource_;
        drawAudioSource_ = 0;
    }

    if (drawLightSource_)
    {
        if (drawLightSource_->isReady())
            drawLightSource_->releaseOpenGl();
        delete drawLightSource_;
        drawLightSource_ = 0;
    }
}

void SceneDebugRenderer::render(const RenderSettings & rs, uint thread, int options)
{
    MO_ASSERT(glReady_, "drawables not defined for SceneDebugRenderer::render()");

    const Mat4&
            proj = rs.cameraSpace().projectionMatrix(),
            cubeView = rs.cameraSpace().cubeViewMatrix(),
            view = rs.cameraSpace().viewMatrix();

    if (options & Scene::DD_CAMERAS)
    for (Camera * o : cameras_)
    {
        const Mat4& trans = o->transformation();
        drawCamera_->renderShader(proj, cubeView * trans, view * trans, trans);
    }

    if (options & Scene::DD_LIGHT_SOURCES)
    for (LightSource * o : lightSources_)
    {
        const Mat4& trans = o->transformation();
        drawLightSource_->renderShader(proj, cubeView * trans, view * trans, trans);
    }
/*
    if (options & Scene::DD_MICROPHONES)
    for (AUDIO::AudioMicrophone * o : microphones_)
    {
        const Mat4& trans = o->transformation(thread, 0);
        drawMicrophone_->renderShader(proj, cubeView * trans, view * trans, trans);
    }

    if (options & Scene::DD_AUDIO_SOURCES)
    for (AUDIO::AudioSource * a : audioSources_)
    {
        const Mat4& trans = a->transformation(thread, 0);
        drawMicrophone_->renderShader(proj, cubeView * trans, view * trans, trans);
    }
*/
}



} // namespace GL
} // namespace MO
