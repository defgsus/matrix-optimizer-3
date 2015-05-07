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
#include <QPainter>

#include "wavetracershader.h"
#include "gl/offscreencontext.h"
#include "gl/framebufferobject.h"
#include "gl/shader.h"
#include "gl/shadersource.h"
#include "gl/screenquad.h"
#include "gl/texture.h"
#include "audio/tool/irmap.h"
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
    void getBuffer(bool doSampleIr, bool doClear); //! locked
    void sampleIr(bool doClearIr); //! not locked, called by getBuffer
    QImage getImage();
    QImage getIrImage(const QSize& s);

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
                * u_SND_COLOR,             // rendermode 3
                * u_DIFFUSE,               // random reflection [0,1]
                * u_DIFFUSE_RND,           // random random reflection [0,1]
                * u_FRESNEL,               // not fresnel really,
                * u_RND_RAY;


    Settings settings, nextSettings;
    LiveSettings liveSettings;
    IrMap::Settings irSettings;
    QString errorString;
    volatile int passCount;
    int passesAdded;
    std::vector<Float> buffer, bufferAdd;
    AUDIO::IrMap irMap;
    //const static qint64 dist_units = 1e+7; //! quantized units per distance unit

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

QString WaveTracerShader::infoString() const
{
    return tr("pass %1/%2; %3")
                   .arg(p_->passCount)
                   .arg(p_->settings.numPasses)
                   .arg(getIrInfo())
    ;
}

uint WaveTracerShader::passCount() const { return p_->passCount; }

uint WaveTracerShader::passesLeft() const { return p_->settings.numPasses - p_->passCount; }

QImage WaveTracerShader::getImage()
{
    return p_->getImage();
}

QImage WaveTracerShader::getIrImage(const QSize& s)
{
    return p_->getIrImage(s);
}

AUDIO::IrMap WaveTracerShader::getIrMap() const
{
    QReadLocker lock(&p_->bufferLock);

    return p_->irMap;
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

const IrMap::Settings& WaveTracerShader::irSettings() const
{
    return p_->irSettings;
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

void WaveTracerShader::setIrSettings(const IrMap::Settings & s)
{
    p_->irSettings = s;
}

void WaveTracerShader::setNumPasses(uint num)
{
    p_->settings.numPasses = num;
}

void WaveTracerShader::Private::defaultSettings()
{
    liveSettings.camera = glm::translate(Mat4(1.f), Vec3(0.f, 0.f, 5.f));
    liveSettings.soundPos = Vec3(0.f);
    liveSettings.soundColor = Vec3(1.f);
    liveSettings.soundRadius = 1.;

    liveSettings.fudge = 0.9f;
    liveSettings.normalEps = 0.0001f;
    liveSettings.maxRayDist = 100.f;
    liveSettings.maxTraceDist = 1000.f;
    liveSettings.micAngle = 180.f;
    liveSettings.reflectivness = 0.9f;
    liveSettings.brightness = 1.f;
    liveSettings.diffuse = 1.f;
    liveSettings.diffuseRnd = 1.f;
    liveSettings.fresnel = 1.f;
    liveSettings.rndRay = 0.01f;

    settings.maxTraceStep = 100;
    settings.maxReflectStep = 5;
    settings.numPasses = 1;
    settings.doPassAverage = false;

    settings.resolution = QSize(128, 128);
    settings.renderMode = RM_WAVE_TRACER;
    settings.userCode =
        "#include <iq/distfunc>\n"
        "#include <dh/hash>\n"
        "#include <noise>\n\n"
        "float DE_room(in vec3 p)\n{\n"
            "\t// noisy surface\n"
            "\t//p += 0.003 * noise3(p*30.);\n\n"
            "\t// two rooms\n"
            "\tfloat d = -sdBox(p, vec3(10.));\n"
            "\td = max(d, -sdBox(p - vec3(0,0,-21), vec3(10.)));\n"
            "\t// connected by window\n"
            "\td = max(d, -sdBox(p - vec3(0,0,-10), vec3(3.)));\n\n"
            "\t// some obstacles\n"
            "\tp = mod(p, 2.) - 1.;\n"
            "\td = min(d, sdBox(p, vec3(0.2)));\n\n"
            "\treturn d;\n}\n\n"
        "float DE_sound(in vec3 p)\n{\n\treturn length(p);\n}\n\n"
        "float DE_reflection(in vec3 p, in vec3 n, in vec3 rd)\n{\n\treturn 1.; // .5 + .5 * hash1(p);\n}\n";

    nextSettings = settings;

    irSettings = irMap.getSettings();
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
    passesAdded = 0;

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
                passCount = 0;
                initGl();
                doRecreate = false;
            }

            // recompile if necessary
            if (doRecompile)
            {
                settings = nextSettings;
                passCount = 0;
                initQuad(); // sets doRecompile
            }

            renderQuad();

            // download texture,
            // sample irMap when correct mode, clear when first pass
            getBuffer(settings.renderMode == RM_WAVE_TRACER, passCount == 0);

            ++passCount;

            emit p->frameFinished();

            msleep(10);
        }
        catch (const Exception& e)
        {
            addError(e.what());

            doStop = true;
        }

        // wait for work
        while (!doStop && passCount >= (int)settings.numPasses)
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
                                            settings.resolution.height(),
                                            gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT);
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
    src->addDefine(QString("#define _NUM_SAMPLES 1"));//.arg(settings.numMultiSamples));

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
    u_DIFFUSE = sh->getUniform("_DIFFUSE", false);
    u_DIFFUSE_RND = sh->getUniform("_DIFFUSE_RND", false);
    u_FRESNEL = sh->getUniform("_FRESNEL", false);
    u_RND_RAY = sh->getUniform("_RND_RAY", false);

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
    u_SNDSRC->set(liveSettings.soundPos);
    u_SNDSRC->floats[3] = liveSettings.soundRadius;
    u_FUDGE->floats[0] = liveSettings.fudge;
    u_EPSILON->floats[0] = liveSettings.normalEps;
    u_DAMPING->floats[0] = liveSettings.reflectivness;
    u_MAX_TRACE_DIST->floats[0] = liveSettings.maxRayDist;
    u_MAX_TRACE_DIST->floats[1] = liveSettings.maxTraceDist;

    if (u_transformation)
        u_transformation->set(liveSettings.camera);
    if (u_RND_RAY)
        u_RND_RAY->floats[0] = liveSettings.rndRay;
    if (u_DIFFUSE)
        u_DIFFUSE->floats[0] = liveSettings.diffuse;
    if (u_DIFFUSE)
        u_DIFFUSE_RND->floats[0] = liveSettings.diffuseRnd;
    if (u_FRESNEL)
        u_FRESNEL->floats[0] = liveSettings.fresnel;
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

void WaveTracerShader::Private::getBuffer(bool doSampleIr, bool doClear)
{
    context->finish();

    QWriteLocker lock(&bufferLock);

    size_t size = settings.resolution.width() * settings.resolution.height() * 4;
    if (buffer.size() != size)
        buffer.resize(size);

    fbo->colorTexture()->bind();
    // single pass
    fbo->colorTexture()->download(&buffer[0], gl::GL_RGBA, gl::GL_FLOAT);

    // add passes
    if (settings.doPassAverage)
    {
        bool doCl = doClear;
        if (bufferAdd.size() != size)
        {
            bufferAdd.resize(size);
            doCl = true;
        }
        if (doCl)
        {
            for (auto & i : bufferAdd) i = 0.f;
            passesAdded = 0;
        }

        const Float inv = 1.f / std::max(1, (int)passesAdded + 1);
        for (size_t i=0; i<size; ++i)
        {
            // add
            bufferAdd[i] += buffer[i];
            // get average
            buffer[i] = bufferAdd[i] * inv;
        }
        ++passesAdded;

    }

    if (doSampleIr)
        sampleIr(doClear);
}


void WaveTracerShader::Private::sampleIr(bool doClearIr)
{
    // buffer has been calculated?
    size_t size = settings.resolution.width() * settings.resolution.height() * 4;
    if (buffer.size() != size)
        return;

    if (doClearIr)
        irMap.clear();

    irMap.setSettings(irSettings);

    //Float phase = 0.;
    for (auto ptr = buffer.begin(); ptr != buffer.end(); ptr += 4)
    {
        Float   amp = ptr[0],
                dist = ptr[1];
        const short int
                count = ptr[2] + 0.001;

//        if (count == 0)
//            amp *= liveSettings.directAmp;

        if (amp < 0.0000001)
            continue;

//        if (liveSettings.doFlipPhase && (count & 1) == 1)
//            amp = -amp;
        //amp = amp * sin(dist / 330.f * 6.28f * 60.f + phase);

        irMap.addSample(dist, amp, count);
    }
}

QString WaveTracerShader::getIrInfo() const
{
    QReadLocker lock(&p_->bufferLock);

    return p_->irMap.getInfo();
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

QImage WaveTracerShader::Private::getIrImage(const QSize& res)
{
    QReadLocker lock(&bufferLock);

    return irMap.getImage(res);

    /*
    QImage img(res, QImage::Format_ARGB32);
    img.fill(Qt::black);

    if (irMap.empty())
        return img;

    QPainter p(&img);
    //p.begin(&img);

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(255,255,255,50)));

//    MO_PRINT("dist " << maxSampledDist << "; amp " << maxSampledAmp);

    // draw
    for (auto it = irMap.begin(); it != irMap.end(); ++it)
    {
        QPointF p0(qreal(it->first) / maxSampledDist * img.width(), .5 * img.height()),
                p1(p0.x(), (it->second / maxSampledAmp * -.5 + .5) * img.height());
        p.drawLine(p0, p1);
    }

    p.end();
    return img;
    */
}

namespace {

    /** Little thread for rendering images */
    class WaveTracerThread_ : public QThread
    {
    public:
        WaveTracerThread_(QObject * o) : QThread(o) { }

        void run()
        {
            img = irMap.getImage(res);
        }

        QSize res;
        QImage img;
        AUDIO::IrMap irMap;
    };
}

void WaveTracerShader::requestIrImage(const QSize &s)
{
    auto thread = new WaveTracerThread_(this);

    thread->res = s;

    {
        QReadLocker lock(&p_->bufferLock);
        p_->irMap.setSettings(p_->irSettings);
        thread->irMap = p_->irMap;
    }

    connect(thread, &WaveTracerThread_::finished, [=]()
    {
        //MO_PRINT("ended ir image thread");
        emit finishedIrImage(thread->img);
        thread->deleteLater();
    });

    //MO_PRINT("START ir image thread");
    thread->start();
}


} // namespace AUDIO
} // namespace MO
