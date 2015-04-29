/** @file wavetracershader.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.04.2015</p>
*/

#include <vector>
#include <map>
#include <atomic>

#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

#include "wavetracershader.h"
#include "gl/offscreencontext.h"
#include "gl/framebufferobject.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/screenquad.h"
#include "gl/texture.h"
#include "io/currentthread.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {


struct WaveTracerShader::Private
{
    Private(WaveTracerShader*p)
        : p         (p)
        , context   (0)
        , fbo       (0)
        , quad      (0)
        , maxPasses (1)
    {
        defaultSettings();
    }

    void addError(const QString& e) { errorString += e; doStop = true; MO_DEBUG("WaveTracerShader: " << e); }

    void run();
    void defaultSettings();
    void initGl();
    void releaseGl();
    void initQuad(); //! (re-)creates quad instance and (re-)compiles shader, throws
    void releaseQuad();
    void renderQuad();
    void getBuffer();
    QImage getImage();

    WaveTracerShader * p;

    GL::OffscreenContext * context;
    GL::FrameBufferObject * fbo;
    GL::ScreenQuad * quad;
    GL::Uniform * u_resolution,
                * u_transformation,        // camera
                * u_SNDSRC,                // xyz = pos, w = radius/thickness
                * u_FUDGE,                 // ray step precision
                * u_MIC_ANGLE,             // microphone opening angle
                * u_MAX_TRACE_DIST,
                * u_DAMPING,               // global reflection amount
                * u_BRIGHTNESS,            // amount of visual light/sound, rendermode 1,2
                * u_EPSILON,
                * u_PASS_NUMBER,           // number of pass (for multi-sampling)
                * u_SND_COLOR;             // rendermode 3

    Settings settings, nextSettings;
    LiveSettings liveSettings;
    QString errorString;
    volatile int passCount, maxPasses;
    std::vector<Float> buffer;
    std::map<Float, Float> ampMap;

    QImage image;
    QReadWriteLock bufferLock;

    volatile bool doStop, doRecompile, doRecreate;
};


WaveTracerShader::WaveTracerShader(QObject * o)
    : QThread   (o),
      p_        (new Private(this))
{
}

WaveTracerShader::~WaveTracerShader()
{
    delete p_;
}

void WaveTracerShader::run()
{
    p_->run();
}

void WaveTracerShader::stop()
{
    p_->doStop = true;
    wait();
}

QImage WaveTracerShader::getImage()
{
    return p_->getImage();
}

const WaveTracerShader::LiveSettings& WaveTracerShader::liveSettings() const
{
    return p_->liveSettings;
}

const WaveTracerShader::Settings& WaveTracerShader::settings() const
{
    /* XXX This hack is only because of WaveTracerDialog::Private::updateVisibility */
    return p_->doRecompile
            ? p_->nextSettings
            : p_->settings;
}

const QString& WaveTracerShader::errorString() const { return p_->errorString; }

void WaveTracerShader::setLiveSettings(const LiveSettings & s)
{
    p_->liveSettings = s;
    p_->passCount = 0;
}

void WaveTracerShader::setSettings(const Settings& s)
{
    p_->nextSettings = s;
    p_->doRecompile = true;
    if (s.resolution != p_->settings.resolution)
        p_->doRecreate = true;
    p_->passCount = 0;
}




void WaveTracerShader::Private::defaultSettings()
{
    liveSettings.camera = glm::translate(Mat4(1.f), Vec3(0.f, 0.f, 5.f));
    liveSettings.soundPos = Vec3(0.f);
    liveSettings.soundColor = Vec3(1.f);
    liveSettings.soundRadius = 1.;

    liveSettings.fudge = 0.9f;
    liveSettings.normalEps = 0.0001f;
    liveSettings.maxTraceDist = 100.f;
    liveSettings.micAngle = 180.f;
    liveSettings.reflectivness = 0.9f;
    liveSettings.brightness = 1.f;

    settings.maxTraceStep = 100;
    settings.maxReflectStep = 5;
    settings.numMultiSamples = 10;

    settings.resolution = QSize(128, 128);
    settings.renderMode = RM_WAVE_TRACER;
    settings.userCode =
        "#include <iq/distfunc>\n\n"
        "float DE_room(in vec3 p)\n{\n\t//inverted box as room\n\treturn -sdBox(p, vec3(10.));\n}\n\n"
        "float DE_sound(in vec3 p)\n{\n\treturn length(p);\n}\n\n"
        "float DE_reflection(in vec3 p, in vec3 n)\n{\n\treturn 1.;\n}\n";

    nextSettings = settings;
}

void WaveTracerShader::Private::run()
{
    static std::atomic_int _count;
    setCurrentThreadName(QString("WT%1").arg(++_count));

    settings = nextSettings;
    doStop = false;
    doRecreate = false;
    errorString.clear();
    passCount = 0;

    initGl();

    if (doStop)
        return;

    fbo->bind();

    while (!doStop)
    {
        try
        {
            // recreate all
            if (doRecreate)
            {
                releaseGl();
                settings = nextSettings;
                initGl();
            }

            // recompile if necessary
            if (doRecompile)
            {
                settings = nextSettings;
                initQuad();
            }

            renderQuad();

            getBuffer();

            ++passCount;

            emit p->frameFinished();
        }
        catch (const Exception& e)
        {
            addError(e.what());

            doStop = true;
        }

        // wait for work
        while (!doStop && passCount >= maxPasses)
            msleep(50);
    }

    releaseGl();
}


void WaveTracerShader::Private::initGl()
{
    try
    {
        if (!context)
        {
            context = new GL::OffscreenContext;
            if (!context->createGl())
            {
                addError(tr("Could not create offscreen opengl context"));

                delete context;
                context = 0;
            }
        }

        if (!fbo)
        {
            fbo = new GL::FrameBufferObject(settings.resolution.width(),
                                            settings.resolution.height(), gl::GL_RGBA, gl::GL_FLOAT);
            fbo->create();
        }

        initQuad();
    }
    catch (const Exception& e)
    {
        addError(e.what());
        releaseGl();
    }
}

void WaveTracerShader::Private::initQuad()
{
    if (!quad)
    {
        quad = new GL::ScreenQuad("wavetracer");
        doRecompile = true;
    }

    // -- prepare source --

    auto src = new GL::ShaderSource;
    src->loadVertexSource(":/shader/wavetracer.vert");
    src->loadFragmentSource(":/shader/wavetracer.frag");

    src->addDefine(QString("#define _RENDER_MODE %1").arg(settings.renderMode));
    src->addDefine(QString("#define _MAX_TRACE_STEPS %1").arg(settings.maxTraceStep));
    src->addDefine(QString("#define _MAX_REFLECT %1").arg(settings.maxReflectStep));
    src->addDefine(QString("#define _NUM_SAMPLES %1").arg(settings.numMultiSamples));

    src->replace("//!mo_user_functions!", settings.userCode, true);

    src->pasteDefaultIncludes();

    //MO_PRINT("compiling");
    //MO_PRINT(src->fragmentSource());
    //MO_PRINT(settings.userCode);

    if (quad->isCreated())
        quad->release();

    quad->create(src);

    // -- get uniforms --

    GL::Shader * sh = quad->shader();
    u_resolution = sh->getUniform("u_resolution", false);
    u_transformation = sh->getUniform("u_transformation", false);
    u_SNDSRC = sh->getUniform("_SNDSRC", true);
    u_FUDGE = sh->getUniform("_FUDGE", true);
    u_MIC_ANGLE = sh->getUniform("_MIC_ANGLE", false);
    u_MAX_TRACE_DIST = sh->getUniform("_MAX_TRACE_DIST", true);
    u_DAMPING = sh->getUniform("_DAMPING", true);
    u_BRIGHTNESS = sh->getUniform("_BRIGHTNESS", false);
    u_EPSILON = sh->getUniform("_EPSILON", true);
    u_PASS_NUMBER = sh->getUniform("_PASS_NUMBER", false);
    u_SND_COLOR = sh->getUniform("_SND_COLOR", false);

    // --- set defaults ---

    if (u_transformation)
        u_transformation->setAutoSend(true);

    if (u_resolution)
        u_resolution->setFloats(settings.resolution.width(),
                                settings.resolution.height(),
                                1.f / std::max(1, settings.resolution.width()),
                                1.f / std::max(1, settings.resolution.height()));

    doRecompile = false;
}

void WaveTracerShader::Private::releaseQuad()
{
    if (quad && quad->isCreated())
        quad->release();
    delete quad;
    quad = 0;
}

void WaveTracerShader::Private::releaseGl()
{
    releaseQuad();

    if (fbo)
        fbo->release();
    delete fbo;
    fbo = 0;

    if (context)
        context->destroyGl();
    delete context;
    context = 0;
}

void WaveTracerShader::Private::renderQuad()
{
    if (u_transformation)
        u_transformation->set(liveSettings.camera);
    u_SNDSRC->set(liveSettings.soundPos);
    u_SNDSRC->floats[3] = liveSettings.soundRadius;
    u_FUDGE->floats[0] = liveSettings.fudge;
    u_EPSILON->floats[0] = liveSettings.normalEps;
    u_DAMPING->floats[0] = liveSettings.reflectivness;
    u_MAX_TRACE_DIST->floats[0] = liveSettings.maxTraceDist;

    if (u_BRIGHTNESS)
        u_BRIGHTNESS->floats[0] = liveSettings.brightness;
    if (u_MIC_ANGLE)
        u_MIC_ANGLE->floats[0] = liveSettings.micAngle;
    if (u_SND_COLOR)
        u_SND_COLOR->set(liveSettings.soundColor);
    if (u_PASS_NUMBER)
        u_PASS_NUMBER->ints[0] = passCount;

    quad->draw(settings.resolution.width(), settings.resolution.height());
}

void WaveTracerShader::Private::getBuffer()
{
    context->finish();

    QWriteLocker lock(&bufferLock);

    size_t size = settings.resolution.width() * settings.resolution.height() * 4;
    if (buffer.size() != size)
        buffer.resize(size);

    fbo->colorTexture()->bind();
    fbo->colorTexture()->download(&buffer[0], gl::GL_RGBA, gl::GL_FLOAT);
}

QImage WaveTracerShader::Private::getImage()
{
    QReadLocker lock(&bufferLock);

    // buffer is not rendered yet?
    size_t size = settings.resolution.width() * settings.resolution.height() * 4;
    if (buffer.size() != size)
        return QImage();

    QImage img(settings.resolution, QImage::Format_RGB32);

    for (int y=0; y<settings.resolution.height(); ++y)
    for (int x=0; x<settings.resolution.width(); ++x)
    {
        const float * pix = &buffer[((settings.resolution.height()-1-y)*settings.resolution.width()+x)*4];
        img.setPixel(x, y, qRgb(
                         255 * std::max(0.f, std::min(1.f, pix[0] )),
                         255 * std::max(0.f, std::min(1.f, pix[1] )),
                         255 * std::max(0.f, std::min(1.f, pix[2] ))
                         ));
    }

    return img;
}

} // namespace AUDIO
} // namespace MO
