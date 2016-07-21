/** @file scenedebugrenderer.cpp

    @brief Renderer for "invisible objects" for Scene

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/31/2014</p>
*/

#include <memory>

#include "SceneDebugRenderer.h"
#include "io/log.h"
#include "object/Scene.h"
#include "object/SoundSource.h"
#include "object/Microphone.h"
#include "object/visual/Camera.h"
#include "object/visual/LightSource.h"
#include "gl/Drawable.h"
#include "gl/ShaderSource.h"
#include "gl/RenderSettings.h"
#include "gl/CameraSpace.h"
#include "gl/Shader.h"
#include "geom/Geometry.h"
#include "geom/ObjLoader.h"
#include "audio/tool/AudioBuffer.h"
#include "audio/spatial/SpatialMicrophone.h"
#include "audio/spatial/SpatialSoundSource.h"

#if 0
#   include "io/log.h"
#   define MO__D(arg__) MO_PRINT("SceneDebugRenderer("<<this<<")::" << arg__)
#else
#   define MO__D(unused__) { }
#endif

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
    GL::Drawable* createConeDrawable(const QString& name);
    void addCoordinates(GEOM::Geometry*) const;
    void render(const RenderSettings & rs, const RenderTime& time, int options);

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
    MO__D("SceneDebugRenderer(" << s << ")");
    p_->scene = s;
}

SceneDebugRenderer::~SceneDebugRenderer()
{
    MO__D("~SceneDebugRenderer()");
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
    MO__D("updateTree()");

    p_->updateTree();
}

void SceneDebugRenderer::initGl()
{
    MO__D("initGl()");

    p_->initGl();
}

void SceneDebugRenderer::releaseGl()
{
    MO__D("releaseGl()");

    p_->releaseGl();
}

void SceneDebugRenderer::render(
        const RenderSettings & rs, const RenderTime& rt, int options)
{
    p_->render(rs, rt, options);
}

void SceneDebugRenderer::Private::updateTree()
{
    cameras = scene->findChildObjects<Camera>(QString(), true);
    lightSources = scene->findChildObjects<LightSource>(QString(), true);
    microphones.clear();
    sounds.clear();

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
        auto snd = new Sound(o);
        sounds.push_back( std::shared_ptr<Sound>(snd) );
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

#if 0
    drawMicrophone = new GL::Drawable("scene_debug_microphone");
    objload.loadFile(":/model/audiosource.obj");
    objload.getGeometry(drawMicrophone->geometry());
    drawMicrophone->geometry()->convertToLines();
    drawMicrophone->geometry()->scale(0.15, 0.15, 0.15);
    addCoordinates(drawMicrophone->geometry());
    drawMicrophone->setShaderSource(src);
    drawMicrophone->createOpenGl();
#else
    drawMicrophone = createConeDrawable("scene_debug_microphone");
#endif

    glReady = true;
}

void SceneDebugRenderer::Private::addCoordinates(GEOM::Geometry * geom) const
{
    const Float len = 1.0;
    geom->setColor(1,0.5,0.5,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(len,0,0));
    geom->setColor(0.5,1,0.5,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(0,len,0));
    geom->setColor(0.5,0.5,1,1);
    geom->addLine(
                geom->addVertexAlways(0,0,0),
                geom->addVertexAlways(0,0,len));

}

/** This builds a cone Geometry with a special attribute used to
    visually approximate the exponential pow(dot()) direction function
    controlled by a shader uniform */
GL::Drawable* SceneDebugRenderer::Private::createConeDrawable(const QString& name)
{
    GL::ShaderSource src;
    src.loadDefaultSource();
    src.addDefine("#define MO_ENABLE_VERTEX_OVERRIDE");
    src.replace("//%mo_override_vert2%",
                 "//#include <transform>\n"
                 "mat4 mo_user_transform()\n"
                 "{\n"
                 "    mat4 m = mat4(1.);\n"
                 "    vec3 cone = a_cone;\n"
                 "    float cexp = u_cone_exp;\n"
                 "    cone.z *= -clamp(cexp-1., -1., 1.);\n"
                 "    cone.xy /= (1. + .1*cexp);\n"
                 "    m[3] += m * vec4(cone, 0.);\n"
                 "    //m = translate(m, cone);\n"
                 "    return m;\n"
                 "}\n"
                 "vec3 mo_modify_position(in vec3 p) { return p; }\n"
                 "void mo_modify_vertex_output() { }\n"
                );
    src.replace("//%user_uniforms%", "uniform float u_cone_exp;\n");

    auto g = new GEOM::Geometry();
    g->setSharedVertices(false);

    g->addAttribute("a_cone", 3);

    auto i0 = g->addVertex(0.f, 0.f, 0.f);
    for (int i=0; i<36; ++i)
    {
        float t = float(i) / 18.f * 3.14159265f;
        Vec3 cone(std::sin(t), std::cos(t), 1.);

        auto i1 = g->addVertex(0.f, 0.f, 0.f);
        g->setAttribute("a_cone", i1, cone.x, cone.y, cone.z);
        g->addLine(i0, i1);
    }
    for (int i=0; i<36; ++i)
        g->addLine(i+1, (((i+1) % 36) + 1));

    addCoordinates(g);

    auto draw = new GL::Drawable(name);
    draw->setGeometry(g);

    draw->setShaderSource(src);
    try
    {
        draw->createOpenGl();
    }
    catch (...)
    {
        delete draw;
        throw;
    }
    return draw;
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

void SceneDebugRenderer::Private::render(
        const RenderSettings & rs, const RenderTime& time, int options)
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
    {
        auto u_cone_exp = drawMicrophone->shader()->getUniform("u_cone_exp");
        for (auto & pm : microphones)
        {
            const Micro * m = pm.get();
            if (!m->object->activeAtAll())
                continue;
            m->trans->setTransformation(m->object->transformation(), 0);
            // calc one sample of transformations
            m->object->calculateMicrophoneTransformation(
                        m->trans, m->mics, time);
            for (const AUDIO::SpatialMicrophone * mic : m->mics)
            {
                if (u_cone_exp)
                    u_cone_exp->floats[0] = mic->directionExponent();
                const Mat4& trans = mic->transformationBuffer()->transformation(0);
                /** @todo avoid unnecessary state changes in multiple
                          calls to Drawable::renderShader */
                drawMicrophone->renderShader(proj, cubeView * trans,
                                             view * trans, trans);
            }
        }
    }

    if (options & Scene::DD_AUDIO_SOURCES)
    for (auto & ps : sounds)
    {
        const Sound * s = ps.get();
        if (!s->object->activeAtAll())
            continue;
        s->trans->setTransformation(s->object->transformation(), 0);
        // calc one sample of transformations
        s->object->calculateSoundSourceTransformation(
                    s->trans, s->snds, time);
        // draw
        for (const AUDIO::SpatialSoundSource * snd : s->snds)
        {
            const Mat4& trans = snd->transformationBuffer()->transformation(0);
            drawAudioSource->renderShader(
                        proj, cubeView * trans, view * trans, trans);
        }

    }

}


/*
import matrixoptimizer as mo
import math

g = mo.geometry()
g.set_shared(False)

i0 = g.add_vertex(0, 0, 0)
for i in range(36):
    t = i / 36. * 3.14159265 * 2.
    cone = mo.Vec(math.sin(t), math.cos(t), 1)

    i1 = g.add_vertex(0,0,0)
    g.set_attribute("a_cone", i1, cone)
    g.add_line(i0, i1)

for i in range(36):
    g.add_line(i+1, ((i+1) % 36 + 1))


#include <transform>

mat4 mo_user_transform()
{
    mat4 m = mat4(1.);
    vec3 cone = a_cone;
    float cexp = 13.;
    cone.z *= clamp(cexp-1., -1., 1.);
    cone.xy /= (1. + .1*cexp);
    m = translate(m, cone);
    return m;
}


*/






} // namespace GL
} // namespace MO
