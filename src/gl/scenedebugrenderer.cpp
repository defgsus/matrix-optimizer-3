/** @file scenedebugrenderer.cpp

    @brief Renderer for "invisible objects" for Scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include <memory>

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
#include "audio/tool/audiobuffer.h"
#include "audio/spatial/spatialmicrophone.h"
#include "audio/spatial/spatialsoundsource.h"

namespace MO {
namespace GL {


struct SceneDebugRenderer::Private
{
    Private(SceneDebugRenderer * r)
        : p                 (r)
        , scene             (0)
        , glReady           (false)
        , drawCamera        (0)
        , drawAudioSource   (0)
        , drawMicrophone    (0)
        , drawLightSource   (0)
    { }

    void updateTree();
    void initGl();
    void releaseGl();
    void addCoordinates(GEOM::Geometry*) const;
    void render(const RenderSettings & rs, uint thread, int options);

    /** struct to fake the microphones of an object */
    struct Micro
    {
        Micro(Object*);
        ~Micro();

        Object * object; //! link to parent
        TransformationBuffer * trans;
        AUDIO::AudioBuffer * buf;
        QList<AUDIO::SpatialMicrophone*> mics;
    };

    /** likewise for sounds */
    struct Sound
    {
        Sound(Object*);
        ~Sound();

        Object * object; //! link to parent
        TransformationBuffer * trans;
        AUDIO::AudioBuffer * buf;
        QList<AUDIO::SpatialSoundSource*> snds;
    };

    SceneDebugRenderer * p;
    Scene * scene;
    QList<Camera*> cameras;
    QList<LightSource*> lightSources;
    QList<std::shared_ptr<Micro>> microphones;
    QList<std::shared_ptr<Sound>> sounds;

    bool glReady;

    GL::Drawable
        * drawCamera,
        * drawAudioSource,
        * drawMicrophone,
        * drawLightSource;
};


SceneDebugRenderer::SceneDebugRenderer(Scene * s)
    : p_        (new Private(this))
{
    p_->scene = s;
}

SceneDebugRenderer::~SceneDebugRenderer()
{
    // only for debugging mainly
    // these objects should be deleted by releaseGl()
    delete p_->drawAudioSource;
    delete p_->drawCamera;
    delete p_->drawMicrophone;
    delete p_->drawLightSource;

    delete p_;
}

bool SceneDebugRenderer::isGlInitialized() const { return p_->glReady; }

void SceneDebugRenderer::updateTree()
{
    p_->updateTree();
}

void SceneDebugRenderer::initGl()
{
    p_->initGl();
}

void SceneDebugRenderer::releaseGl()
{
    p_->releaseGl();
}

void SceneDebugRenderer::render(const RenderSettings & rs, uint thread, int options)
{
    p_->render(rs, thread, options);
}

void SceneDebugRenderer::Private::updateTree()
{
    cameras = scene->findChildObjects<Camera>(QString(), true);
    lightSources = scene->findChildObjects<LightSource>(QString(), true);

    // get all objects with microphones
    QList<Object*> objs = scene->findChildObjects<Object>([](const Object*o)
    {
        return o->numberMicrophones() > 0;
    }, true);

    for (Object * o : objs)
    {
        auto mic = new Micro(o);
        microphones.push_back( std::shared_ptr<Micro>(mic) );
    }

    // sound source objects
    objs = scene->findChildObjects<Object>([](const Object*o)
    {
        return o->numberSoundSources() > 0;
    }, true);

    for (Object * o : objs)
    {
        auto mic = new Sound(o);
        sounds.push_back( std::shared_ptr<Sound>(mic) );
    }


}

SceneDebugRenderer::Private::Micro::Micro(Object * o)
    : object    (o)
    , trans     (new TransformationBuffer(1))
    , buf       (new AUDIO::AudioBuffer(1))
{
    for (uint i=0; i<object->numberMicrophones(); ++i)
    {
        auto mic = new AUDIO::SpatialMicrophone(buf, 44100, i);
        mics.append(mic);
    }
}

SceneDebugRenderer::Private::Micro::~Micro()
{
    for (auto m : mics)
        delete m;
    delete buf;
    delete trans;
}

SceneDebugRenderer::Private::Sound::Sound(Object * o)
    : object    (o)
    , trans     (new TransformationBuffer(1))
    , buf       (new AUDIO::AudioBuffer(1))
{
    for (uint i=0; i<object->numberSoundSources(); ++i)
    {
        auto snd = new AUDIO::SpatialSoundSource(buf, 0);
        snds.append(snd);
    }
}

SceneDebugRenderer::Private::Sound::~Sound()
{
    for (auto s : snds)
        delete s;
    delete buf;
    delete trans;
}


void SceneDebugRenderer::Private::initGl()
{
    GEOM::ObjLoader objload;

    // default wire-frame shader
    GL::ShaderSource src;
    src.loadDefaultSource();

    // --- setup camera drawable ----

    drawCamera = new GL::Drawable("scene_debug_camera");
    objload.loadFile(":/model/camera.obj");
    objload.getGeometry(drawCamera->geometry());
    drawCamera->geometry()->convertToLines();
    drawCamera->geometry()->scale(0.1, 0.1, 0.1);
    addCoordinates(drawCamera->geometry());
    drawCamera->setShaderSource(src);
    drawCamera->createOpenGl();

    // --- setup AudioSource drawable ----

    drawAudioSource = new GL::Drawable("scene_debug_audiosource");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawAudioSource->geometry());
    drawAudioSource->geometry()->convertToLines();
    drawAudioSource->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates(drawAudioSource->geometry());
    drawAudioSource->setShaderSource(src);
    drawAudioSource->createOpenGl();

    // --- setup LightSource drawable ----

    drawLightSource = new GL::Drawable("scene_debug_lightsource");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawLightSource->geometry());
    drawLightSource->geometry()->convertToLines();
    drawLightSource->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates(drawLightSource->geometry());
    drawLightSource->setShaderSource(src);
    drawLightSource->createOpenGl();

    // --- setup Microphone drawable ----

    drawMicrophone = new GL::Drawable("scene_debug_microphone");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawMicrophone->geometry());
    drawMicrophone->geometry()->convertToLines();
    drawMicrophone->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates(drawMicrophone->geometry());
    drawMicrophone->setShaderSource(src);
    drawMicrophone->createOpenGl();

    glReady = true;
}

void SceneDebugRenderer::Private::addCoordinates(GEOM::Geometry * geom) const
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

void SceneDebugRenderer::Private::releaseGl()
{
    glReady = false;

    if (drawCamera)
    {
        if (drawCamera->isReady())
            drawCamera->releaseOpenGl();
        delete drawCamera;
        drawCamera = 0;
    }

    if (drawMicrophone)
    {
        if (drawMicrophone->isReady())
            drawMicrophone->releaseOpenGl();
        delete drawMicrophone;
        drawMicrophone = 0;
    }

    if (drawAudioSource)
    {
        if (drawAudioSource->isReady())
            drawAudioSource->releaseOpenGl();
        delete drawAudioSource;
        drawAudioSource = 0;
    }

    if (drawLightSource)
    {
        if (drawLightSource->isReady())
            drawLightSource->releaseOpenGl();
        delete drawLightSource;
        drawLightSource = 0;
    }
}

void SceneDebugRenderer::Private::render(const RenderSettings & rs, uint thread, int options)
{
    MO_ASSERT(glReady, "drawables not defined for SceneDebugRenderer::render()");

    const Mat4&
            proj = rs.cameraSpace().projectionMatrix(),
            cubeView = rs.cameraSpace().cubeViewMatrix(),
            view = rs.cameraSpace().viewMatrix();

    if (options & Scene::DD_CAMERAS)
    for (Camera * o : cameras)
    {
        if (!o->activeAtAll())
            continue;
        const Mat4& trans = o->transformation();
        drawCamera->renderShader(proj, cubeView * trans, view * trans, trans);
    }

    if (options & Scene::DD_LIGHT_SOURCES)
    for (LightSource * o : lightSources)
    {
        if (!o->activeAtAll())
            continue;
        const Mat4& trans = o->transformation();
        drawLightSource->renderShader(proj, cubeView * trans, view * trans, trans);
    }

    if (options & Scene::DD_MICROPHONES)
    for (auto & pm : microphones)
    {
        const Micro * m = pm.get();
        if (!m->object->activeAtAll())
            continue;
        m->trans->setTransformation(m->object->transformation(), 0);
        SamplePos pos = 0;
        // calc one sample of transformations
        m->object->calculateMicrophoneTransformation(
                    m->trans, m->mics, 1, pos, thread);
        for (const AUDIO::SpatialMicrophone * mic : m->mics)
        {
            const Mat4& trans = mic->transformationBuffer()->transformation(0);
            /** @todo avoid unnecessary state changes in multiple calls to Drawable::renderShader */
            drawMicrophone->renderShader(proj, cubeView * trans, view * trans, trans);
        }
    }

    if (options & Scene::DD_AUDIO_SOURCES)
    for (auto & ps : sounds)
    {
        const Sound * s = ps.get();
        if (!s->object->activeAtAll())
            continue;
        s->trans->setTransformation(s->object->transformation(), 0);
        SamplePos pos = 0;
        // calc one sample of transformations
        s->object->calculateSoundSourceTransformation(
                    s->trans, s->snds, 1, pos, thread);
        // draw
        for (const AUDIO::SpatialSoundSource * snd : s->snds)
        {
            const Mat4& trans = snd->transformationBuffer()->transformation(0);
            drawAudioSource->renderShader(proj, cubeView * trans, view * trans, trans);
        }

    }

}



} // namespace GL
} // namespace MO
