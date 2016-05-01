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
#include "object/util/objectfactory.h"
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
#include "io/time.h"
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
        , sceneEditor   (0)
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
        , stat_image_thread_overhead    (0.)
    { }

    void addError(const QString& e) { if (!errorStr.isEmpty()) errorStr += '\n'; errorStr += e; }
    bool loadScene(const QString&);
    bool initScene();
    bool releaseScene();
    bool renderFrame();
    bool writeImage();
    bool openAudioFile(); //! creates a 32bit float wav for writing
    bool closeAudioFile();
    bool renderAudioFrame();
    bool writeAudioFrame(); //! writes into the open file
    void renderAll();
    bool prepareDir(const QString& dir_or_filename);
    bool writeImage(const QImage&);
    void normalizeAndSplitAudio();

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
    QImage tempImage;

    volatile bool pleaseStop;
    size_t curSample, curFrame;

    // render info
    QTime startTime;
    Double progress,
        stat_image_thread_overhead; // seconds
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

    // render audio/video
    if (p_->rendSet.imageEnable() || p_->rendSet.audioEnable())
    {

        if (!p_->initScene())
        {
            p_->releaseScene();
            return;
        }

        try
        {
            p_->renderAll();
        }
        catch (const Exception& e)
        {
            p_->releaseScene();
            p_->addError(e.what());
            return;
        }

        p_->releaseScene();
    }

    // convert audio files
    if (p_->rendSet.audioEnable() && p_->rendSet.audioSplitEnable())
    {
        try
        {
            p_->normalizeAndSplitAudio();
        }
        catch (const Exception& e)
        {
            p_->releaseScene();
            p_->addError(e.what());
        }
    }
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
        addError(tr("Error loading scene.\n%1").arg(e.what()));
        return false;
    }

    return true;
}

bool DiskRenderer::Private::initScene()
{
    MO_DEBUG("DiskRenderer::initScene()");

    if (!loadScene(sceneFilename))
        return false;

    progress = 0;
    stat_image_thread_overhead = 0.;

    /** that's very loosely the startup routine for preparing a scene.
        @todo misses FrontItems */

    // request lazy resource creation instead of multi-threaded
    scene->setLazyFlag(true);
    // tell everyone that disk rendering is requested (e.g. for quality)
    scene->setRendering(true);

    // init samplerate
    scene->setSceneSampleRate(rendSet.audioConfig().sampleRate());

    // create editor (to make scripts work)
    sceneEditor = new ObjectEditor();
    scene->setObjectEditor(sceneEditor);

    /** @todo When to first run scripts?? */
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
                Double sec = Double(curFrame) / rendSet.imageFps();
                return RenderTime(
                            sec,
                            1. / rendSet.imageFps(),
                            sec * rendSet.audioConfig().sampleRate(),
                            rendSet.audioConfig().sampleRate(),
                            rendSet.audioConfig().bufferSize(),
                            MO_GFX_THREAD);
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
    if (scene)
        scene->releaseRef("render finished"); scene = 0;
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
        gl::glFinish();

        fbo->colorTexture()->bind();
        tempImage = fbo->colorTexture()->toQImage();
        if (tempImage.isNull())
            return false;

        gl::glFinish();

        return writeImage(tempImage);
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
        if (time.elapsed() > 1000 / 2)
        {
            //MO_DEBUG("rendering frame " << f << "/" << rendSet.lengthFrame());
            progress = f * 100.f / rendSet.lengthFrame();
            emit thread->progress(progress);
            if (!tempImage.isNull())
                emit thread->newImage(tempImage);
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
    SamplePos
        curf = (p_->curFrame - p_->rendSet.startFrame());

    QString r;
    QTextStream s(&r);
    s <<   "progress : " << p_->progress << "%"
      << "\nposition : frame " << curf
                         << " / " << p_->rendSet.lengthFrame()
                         << " @ " << p_->rendSet.imageFps() << " fps"
                         << "; time " << time_to_string(p_->rendSet.frame2second(p_->curFrame), true);

    if (p_->rendSet.imageEnable() && p_->renderer && p_->threadPool)
    {
        Double spd = p_->renderer->renderSpeed();
        s << "\nimage time : render " << std::floor(spd/1000.)*1000 << "s";
        if (spd <= .5)
            s << " (" << int(1. / spd) << "fps)";

        s               << "; store average " << p_->threadPool->averageWorkTime() << "s";
        if (p_->stat_image_thread_overhead > 0.001)
        {
            s           << "; overhead " << time_to_string_short(p_->stat_image_thread_overhead);
            Double perc = p_->stat_image_thread_overhead / std::max(0.1, elapsed);
            if (perc >= 100.)
                s       << " (" << std::floor(perc) / 100. << "%)";
        }
        s   << "\nimage threads / que : "
                        << p_->threadPool->numberActiveThreads() << "/" << p_->threadPool->numberThreads()
                        << " " << p_->threadPool->numberWork() << "/" << p_->rendSet.imageNumQue();
    }

    s << "\ntime : estimated " << time_to_string_short(estimated)
            << "; elapsed " << time_to_string_short(elapsed)
            << "; left " << time_to_string_short(estimated - elapsed)
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
    //else
    //    return true;

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
    TimeMessure tm;

    QString fn = rendSet.makeImageFilename(curFrame);

    if (!prepareDir(fn))
        return false;

    // only allow x images in parallel
    if (rendSet.imageNumQue() > 0)
    {
        threadPool->block(rendSet.imageNumQue() - 1);
    }

    threadPool->addWork([this, fn, img]()
    {
        QImageWriter w(fn, rendSet.imageFormatExt().toUtf8());
        w.setQuality(rendSet.imageQuality());
        w.setCompression(rendSet.imageCompression());
        /** @todo expose description in gui */
        w.setDescription(QString("%1: frame %2").arg(versionString()).arg(curFrame));

        /*if (!w.canWrite())
        {
            addError(tr("Can not store the image, settings are wrong?"));
            pleaseStop = true;
        }
        else*/
        if (!w.write(img))
        {
            /** @todo get error signals from image write thread */
            addError(tr("Could not write image '%1'\n%2").arg(fn).arg(w.errorString()));
            pleaseStop = true;
        }
    });

    stat_image_thread_overhead += tm.time();

    return true;
}

void DiskRenderer::Private::normalizeAndSplitAudio()
{
    QString fn = rendSet.makeAudioFilename();
    SF_INFO ininfo;
    ininfo.format = 0;
    SNDFILE * infile = sf_open(fn.toStdString().c_str(), SFM_READ, &ininfo);
    if (!infile)
        MO_IO_ERROR(READ, tr("Could not open audio file\n'%1'\n%2")
                 .arg(fn).arg(sf_strerror((SNDFILE*)0)));

    // get maximum value
    F32 normMul = 1.;
    if (rendSet.audioNormalizeEnable())
    {
        Double maxVal;
        int r = sf_command(infile, SFC_CALC_SIGNAL_MAX, &maxVal, sizeof(maxVal));
        if (r)
            MO_IO_ERROR(READ, tr("Could not calculate maximum in audio file\n'%1'\n%2")
                     .arg(fn).arg(sf_strerror(infile)));
        MO_DEBUG("max amplitude: " << maxVal);
        if (maxVal > 0.)
            normMul = 0.999 / maxVal;
    }

    std::vector<SNDFILE*> files;

    try
    {
        // create sndfiles for each channel
        for (size_t i=0; i<(size_t)ininfo.channels; ++i)
        {
            SF_INFO info;
            info.channels = 1;
            info.samplerate = ininfo.samplerate;
            info.frames = ininfo.frames;
            info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24;

            QString fn = rendSet.makeAudioFilename(i);

            // open file
            SNDFILE * file = sf_open(fn.toStdString().c_str(), SFM_WRITE, &info);
            if (!file)
               MO_IO_ERROR(WRITE, tr("Could not open file for writing audio\n'%1'\n%2")
                                    .arg(fn).arg(sf_strerror((SNDFILE*)0)));
            files.push_back(file);
        }

        size_t bsize = 1024 * 16;
        std::vector<F32> data(bsize * ininfo.channels),
                         dataout(bsize);

        // split data
        for (size_t i=0; i<(size_t)ininfo.frames; i += bsize)
        {
            // read a chunk - interlaced
            size_t left = std::min(bsize, size_t(ininfo.frames) - i);
            size_t r = sf_readf_float(infile, &data[0], left);
            if (r != left)
                MO_IO_ERROR(READ, tr("Could not read from audio file\n'%1'\n%2")
                                    .arg(fn).arg(sf_strerror(infile)));

            // deinterlace (and normalize)
            for (size_t k=0; k<(size_t)ininfo.channels; ++k)
            {
                for (size_t j=0; j<left; ++j)
                    dataout[j] = data[j * ininfo.channels + k] * normMul;

                r = sf_writef_float(files[k], &dataout[0], left);
                if (r != left)
                    MO_IO_ERROR(WRITE, tr("Could not write to audio file\n'%1'\n%2")
                                        .arg(rendSet.makeAudioFilename(k))
                                        .arg(sf_strerror(files[k])));
            }

            emit thread->progress(i * 100 / ininfo.frames);
        }

        for (auto f : files)
            sf_close(f);
        sf_close(infile);
    }
    catch (Exception& e)
    {
        for (auto f : files)
            sf_close(f);
        sf_close(infile);
        throw;
    }
}


} // namespace MO
