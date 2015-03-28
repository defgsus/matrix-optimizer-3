/** @file diskrenderer.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/27/2015</p>
*/

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
        , scene         (0)
        , context       (0)
        , renderer      (0)
        , audio         (0)
        , pleaseStop    (false)
        , curSample     (0)
        , curFrame      (0)
    { }

    void addError(const QString& e) { if (!errorStr.isEmpty()) errorStr += '\n'; errorStr += e; }
    bool loadScene(const QString&);
    bool initScene();
    bool releaseScene();
    bool renderFrame();
    bool writeImage();
    void renderAll();
    bool prepareDir(const QString& dir_or_filename);
    bool writeImage(const QImage&);

    DiskRenderer * thread;
    DiskRenderSettings rendSet;
    Scene * scene;
    QString sceneFilename;
    ObjectEditor * sceneEditor;
    GL::OffscreenContext * context;
    GL::SceneRenderer * renderer;
    AudioEngine * audio;
    QString errorStr;

    volatile bool pleaseStop;
    size_t curSample, curFrame;
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

    p_->renderAll();

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

    scene->setLazyFlag(true);

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

    audio = new AudioEngine();
    audio->setScene(scene, rendSet.audioConfig(), MO_AUDIO_THREAD);

    return true;
}

bool DiskRenderer::Private::releaseScene()
{
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
        renderer->render();
        return true;
    }
    catch (const Exception& e)
    {
        addError(tr("Error rendering frame %1\n%2").arg(curFrame).arg(e.what()));
        return false;
    }
}

bool DiskRenderer::Private::writeImage()
{
    if (!scene)
        return false;

    auto fbo = scene->fboMaster(MO_GFX_THREAD);
    if (!fbo || !fbo->colorTexture())
        return false;

    QImage img = fbo->colorTexture()->getImage();
    if (img.isNull())
        return false;

    return writeImage(img);
}

void DiskRenderer::Private::renderAll()
{
    MO_DEBUG("DiskRenderer::renderAll()");

    pleaseStop = false;
    curFrame = 0;

    // Request lazy creation of all gl resources
    renderFrame();

    QTime time;
    time.start();

    size_t f = 0;
    while (f < rendSet.lengthFrame() && thread->ok() && !pleaseStop)
    {
        curFrame = f + rendSet.startFrame();
        ++f;

        renderFrame();
        writeImage();
        //writeImage(img);

        // emit progress every ms..
        if (time.elapsed() > 1000 / 5)
        {
            MO_DEBUG("rendering frame " << f << "/" << rendSet.lengthFrame());
            emit thread->progress(f * 100 / rendSet.lengthFrame());
            time.start();
        }
    }
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

    QImageWriter w(fn, rendSet.imageFormatExt().toUtf8());
    /** @todo expose these in gui */
    w.setQuality(75);
    w.setDescription(QString("%1 frame %2").arg(versionString()).arg(curFrame));

    if (!w.write(img))
    {
        addError(tr("Could not write image '%1'\n%2").arg(fn).arg(w.errorString()));
        return false;
    }
    return true;
}

} // namespace MO
