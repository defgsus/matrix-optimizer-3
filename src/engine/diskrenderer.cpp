/** @file diskrenderer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/27/2015</p>
*/

#include <sndfile.h>

#include <QDir>
#include <QImage>
#include <QImageWriter>
#include <QTime>

#include "diskrenderer.h"
#include "audioengine.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/objectfactory.h"
#include "object/util/objecteditor.h"
#include "gl/offscreencontext.h"
#include "gl/scenerenderer.h"
#include "gl/compatibility.h"
#include "gl/framebufferobject.h"
#include "gl/texture.h"
#include "projection/projectionsystemsettings.h"
#include "tool/stringmanip.h"
#include "tool/threadpool.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "io/filemanager.h"
#include "io/diskrendersettings.h"
#include "io/currentthread.h"
#include "io/settings.h"
#include "io/version.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

struct DiskRenderer::Private
{
    Private(DiskRenderer * t)
        : thread        (t)
        , threadPool    (0)
        , scene         (0)
        , context       (0)
        , renderer      (0)
        , audio         (0)
        , bufIn         (0)
        , bufOut        (0)
        , sndFile       (0)
        , pleaseStop    (false)
        , curSample     (0)
        , curFrame      (0)
        , progress      (0)
    { }

    void addError(const QString& e) { if (!errorStr.isEmpty()) errorStr += '\n'; errorStr += e; }
    bool loadScene(const QString&);
    bool initScene();
    bool releaseScene();
    bool renderFrame();
    bool writeImage();
    bool openAudioFile(); // creates a 32bit float wav for writing
    bool closeAudioFile();
    bool renderAudioFrame();
    bool writeAudioFrame(); // writes into the open file
    void renderAll();
    bool prepareDir(const QString& dir_or_filename);
    bool writeImage(const QImage&);

    DiskRenderer * thread;
    ThreadPool * threadPool;
    DiskRenderSettings rendSet;
    Scene * scene;
    QString sceneFilename;
    ObjectEditor * sceneEditor;
    GL::OffscreenContext * context;
    GL::SceneRenderer * renderer;
    AudioEngine * audio;
    AUDIO::AudioBuffer * bufIn, * bufOut;
    //AUDIO::SoundFile * soundFile;
    SNDFILE * sndFile;
    QString errorStr;

    volatile bool pleaseStop;
    size_t curSample, curFrame;

    // render info
    QTime startTime;
    Double progress;
};


DiskRenderer::DiskRenderer(QObject *parent)
    : QThread       (parent)
    , p_            (new Private(this))
{
    MO_DEBUG("DiskRenderer::" << this);
}

DiskRenderer::~DiskRenderer()
{
    delete p_;
}

bool DiskRenderer::ok() const { return p_->errorStr.isEmpty(); }

QString DiskRenderer::errorString() const { return p_->errorStr; }

const DiskRenderSettings& DiskRenderer::settings() const
{
    return p_->rendSet;
}

void DiskRenderer::setSettings(const DiskRenderSettings & s)
{
    p_->rendSet = s;
}

void DiskRenderer::setSceneFilename(const QString &fn)
{
    p_->sceneFilename = fn;
}

/*void DiskRenderer::setScene(Scene * scene)
{
    p_->scene = scene;
}*/

bool DiskRenderer::loadScene(const QString &fn)
{
    return p_->loadScene(fn);
}

void DiskRenderer::run()
{
    setCurrentThreadName("RENDER");

    if (!p_->initScene())
    {
        p_->releaseScene();
        return;
    }

    try
    {
        p_->renderAll();
    }
    catch (...)
    {
        p_->releaseScene();
        throw;
    }

    p_->releaseScene();
}

void DiskRenderer::stop()
{
    p_->pleaseStop = true;
    wait();
}

bool DiskRenderer::Private::loadScene(const QString& fn)
{
    MO_DEBUG("DiskRenderer::loadScene(" << fn << ")");

    try
    {
        scene = ObjectFactory::loadScene(fn);
    }
    catch (const Exception& e)
    {
        addError(tr("Error loading scene.\n").arg(e.what()));
        return false;
    }

    return true;
}

bool DiskRenderer::Private::initScene()
{
    MO_DEBUG("DiskRenderer::initScene()");

    if (!loadScene(sceneFilename))
        return false;

    /** that's very loosely the startup routine for preparing a scene.
        @todo misses FrontItems */

    // request lazy resource creation instead of multi-threaded
    scene->setLazyFlag(true);

    // init samplerate
    scene->setSceneSampleRate(rendSet.audioConfig().sampleRate());

    // create editor (to make scripts work)
    sceneEditor = new ObjectEditor();
    scene->setObjectEditor(sceneEditor);

    scene->runScripts();

    // projection settings
    scene->setProjectionSettings(MO::settings()->getDefaultProjectionSettings());
    scene->setProjectorIndex(MO::settings()->clientIndex());

    // check for local filenames

    IO::FileList files;
    scene->getNeededFiles(files);

    IO::fileManager().addFilenames(files);
    IO::fileManager().acquireFiles();
#ifndef NDEBUG
    IO::fileManager().dumpStatus();
#endif


    // -- setup gl context --
    if (rendSet.imageEnable())
    {
        try
        {
            renderer = new GL::SceneRenderer();
            context = renderer->createOffscreenContext();

    #ifndef NDEBUG
            GL::Properties prop;
            context->makeCurrent();
            prop.getProperties();
            MO_DEBUG("Offscreen Context:\n" + prop.toString());
    #endif

            renderer->setScene(scene);
            renderer->setTimeCallback([this]()
            {
                return Double(curFrame) / rendSet.imageFps();
            });
            scene->setResolution(QSize(rendSet.imageWidth(), rendSet.imageHeight()));

        }
        catch (const Exception& e)
        {
            addError(tr("Could not create OpenGL context\n%1").arg(e.what()));
            return false;
        }

        if (!threadPool)
            threadPool = new ThreadPool();
        threadPool->start(rendSet.imageNumThreads());
    }

    // --- setup audio ---
    if (rendSet.audioEnable())
    {
        const auto conf = rendSet.audioConfig();

        audio = new AudioEngine();
        audio->setScene(scene, conf, MO_AUDIO_THREAD);

        bufIn = new AUDIO::AudioBuffer(conf.bufferSize() * conf.numChannelsIn(), 1);
        bufOut = new AUDIO::AudioBuffer(conf.bufferSize() * conf.numChannelsOut(), 1);
    }

    return true;
}

bool DiskRenderer::Private::releaseScene()
{
    if (threadPool)
        threadPool->stop();

    // release gl resources
    if (scene)
    {
        try { scene->kill(); }
#ifndef NDEBUG
        catch (const Exception& e)
            { MO_DEBUG("DiskRenderer::releaseScene() ERROR:\n" << e.what()); }
#else
        catch (...) { }
#endif
    }

    delete bufIn; bufIn = 0;
    delete bufOut; bufOut = 0;

    delete audio; audio = 0;
    delete renderer; renderer = 0;
    //delete context; context = 0;
    delete scene; scene = 0;
    delete sceneEditor; sceneEditor = 0;

    return true;
}

bool DiskRenderer::Private::renderFrame()
{
    try
    {
        renderer->setSize(QSize(rendSet.imageWidth(), rendSet.imageHeight()));
        renderer->render(false);
        return true;
    }
    catch (const Exception& e)
    {
        addError(tr("Error rendering frame %1\n%2").arg(curFrame).arg(e.what()));
        return false;
    }
}

bool DiskRenderer::Private::openAudioFile()
{
    QString fn = rendSet.makeAudioFilename();
    if (!prepareDir(fn))
        return false;

    // create sndfile info struct
    SF_INFO info;
    info.channels = rendSet.audioConfig().numChannelsOut();
    info.samplerate = rendSet.audioConfig().sampleRate();
    info.frames = rendSet.lengthSample();
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    // open file
    sndFile = sf_open(fn.toStdString().c_str(), SFM_WRITE, &info);
    if (!sndFile)
    {
       addError(tr("Could not open file for writing audio '%1'\n%2")
                .arg(fn).arg(sf_strerror((SNDFILE*)0)));
       return false;
    }
    return true;
}

bool DiskRenderer::Private::closeAudioFile()
{
    if (sf_close(sndFile) != 0)
    {
        addError(tr("Could not close the audio file\n%1").arg(sf_strerror(sndFile)));
        return false;
    }
    return true;
}

bool DiskRenderer::Private::renderAudioFrame()
{
    if (!audio)
        return true;

    try
    {
        const SamplePos next_frame = rendSet.frame2sample(curFrame + 1);
        while (audio->pos() < next_frame)
        {
            //MO_PRINT("render " << audio->pos());
            audio->processForDevice(bufIn->readPointer(), bufOut->writePointer());
            bufOut->nextBlock();
            if (!writeAudioFrame())
                return false;
        }
    }
    catch (const Exception& e)
    {
        addError(QString("Audio error: %1").arg(e.what()));
        return false;
    }

    return true;
}

bool DiskRenderer::Private::writeAudioFrame()
{
#if 0
    if (!soundFile)
    {
        soundFile = AUDIO::SoundFileManager::createSoundFile(
                        rendSet.audioConfig().numChannelsOut(),
                        rendSet.audioConfig().sampleRate());
    }

    soundFile->appendDeviceData(bufOut->readPointer(), rendSet.audioConfig().bufferSize());
#else
    if (!audio)
        return true;

    if (!sndFile)
    {
        if (!openAudioFile())
            return false;
    }

    uint len = rendSet.audioConfig().bufferSize();
    uint e = sf_writef_float(sndFile, bufOut->readPointer(), len);

    if (e != len)
    {
        MO_IO_ERROR(READ, "could not write all of soundfile\n"
                    "expected " << len << " frames, got " << e );
    }
#endif
    return true;
}

bool DiskRenderer::Private::writeImage()
{
    if (!scene)
        return false;

    auto fbo = scene->fboMaster(MO_GFX_THREAD);
    if (!fbo || !fbo->colorTexture())
        return false;

    try
    {
        renderer->context()->makeCurrent();
        gl::glFlush();
        //msleep(100);

        fbo->colorTexture()->bind();
        QImage img = fbo->colorTexture()->getImage();
        if (img.isNull())
            return false;
        return writeImage(img);
    }
    catch (const Exception& e)
    {
        errorStr += "Error on retrieving framebuffer image.\n";
        errorStr += e.what();
        return false;
    }
}


void DiskRenderer::Private::renderAll()
{
    MO_DEBUG("DiskRenderer::renderAll()");

    pleaseStop = false;
    curFrame = 0;

    // Request lazy creation of all gl resources
    if (renderer)
        renderFrame();

    // seek audio engine to start pos
    if (audio)
        audio->seek(rendSet.startSample());

    QTime time;
    startTime.start();
    time.start();

    size_t f = 0;
    while (f < rendSet.lengthFrame() && thread->ok() && !pleaseStop)
    {
        curFrame = f + rendSet.startFrame();
        ++f;

        renderAudioFrame();
        if (renderer)
        {
            renderFrame();
            writeImage();
        }

        // emit progress every ms..
        if (time.elapsed() > 1000 / 5)
        {
            //MO_DEBUG("rendering frame " << f << "/" << rendSet.lengthFrame());
            progress = f * 100.f / rendSet.lengthFrame();
            emit thread->progress(progress);
            time.start();
        }
    }

    if (sndFile)
        closeAudioFile();
}

QString DiskRenderer::progressString() const
{
    Double
        elapsed = 0.001 * p_->startTime.elapsed(),
        estimated = elapsed / std::max(0.001, p_->progress) * 100.;

    QString r;
    QTextStream s(&r);
    s <<   "progress : " << p_->progress << "%"
      << "\nframe : " << (p_->curFrame - p_->rendSet.startFrame())
                         << " / " << p_->rendSet.lengthFrame()
                         << " @ " << p_->rendSet.imageFps() << " fps"
      << "\nimage threads / que : "
                        << p_->threadPool->numberActiveThreads() << "/" << p_->threadPool->numberThreads()
                        << " " << p_->threadPool->numberWork() << "/" << p_->rendSet.imageNumQue()
      << "\ntime elapsed : " << time_to_string(elapsed)
      << "\ntime estimated : " << time_to_string(estimated)
      << "\ntime to go : " << time_to_string(estimated - elapsed)
         ;
    return r;
}

bool DiskRenderer::Private::prepareDir(const QString& dir1)
{
    // strip filename away
    QString dir = dir1;
    int idx = dir.lastIndexOf('/');
    if (idx < 0)
        idx = dir.lastIndexOf('\\');
    if (idx >= 0)
    {
        dir = dir.left(idx);
    }
    else
        return true;

//    MO_DEBUG("preparing dir [" << dir << "]");

    if (dir.isEmpty())
        return true;

    if (!QDir(".").mkpath(dir))
    {
        addError(tr("Could not create directory for '%1'")
                 .arg(dir));
        return false;
    }
    return true;
}

bool DiskRenderer::Private::writeImage(const QImage& img)
{
    QString fn = rendSet.makeImageFilename(curFrame);
    if (!prepareDir(fn))
        return false;

    // only allow x images in parallel
    if (rendSet.imageNumQue() > 0)
        threadPool->block(rendSet.imageNumQue() - 1);

    threadPool->addWork([this, fn, img]()
    {
        QImageWriter w(fn, rendSet.imageFormatExt().toUtf8());
        w.setQuality(rendSet.imageQuality());
        w.setCompression(rendSet.imageCompression());
        /** @todo expose description in gui */
        w.setDescription(QString("%1: frame %2").arg(versionString()).arg(curFrame));

        if (!w.canWrite())
        {
            //addError(tr("Can not store the image, settings are wrong?"));
            //return false;
        }
        else
        if (!w.write(img))
        {
            /** @todo get error signals from image write thread */
            //addError(tr("Could not write image '%1'\n%2").arg(fn).arg(w.errorString()));
            //return false;
        }
    });
    return true;
}

} // namespace MO
