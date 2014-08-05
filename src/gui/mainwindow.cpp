/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/


#include <QDebug>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QTreeView>
#include <QCloseEvent>
#include <QTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QStatusBar>
#include <QTimer>
#include <QLabel>
#include <QThread>
#include <QDesktopWidget>

#include "mainwindow.h"
#include "projectorsetupwidget.h"
#include "timeline1dview.h"
#include "timeline1drulerview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "gui/painter/grid.h"
#include "gui/qobjectinspector.h"
#include "gui/objecttreeview.h"
#include "gui/objectview.h"
#include "gui/sequencefloatview.h"
#include "gui/sequencer.h"
#include "gui/widget/spacer.h"
#include "gui/util/scenesettings.h"
#include "gui/audiodialog.h"
#include "gui/geometrydialog.h"
#include "model/objecttreemodel.h"
#include "io/datastream.h"
#include "io/files.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "io/error.h"
#include "io/log.h"
#include "io/memory.h"
#include "io/settings.h"
#include "io/application.h"
#include "engine/renderer.h"
#include "gl/texture.h"

#include "object/objectfactory.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {


static QString windowTitleString("Matrix Optimizer 3.0");



class TestThread : public QThread
{
public:
    TestThread(Scene * scene, QObject * parent)
        :   QThread(parent),
          scene_  (scene),
          stop_   (false)
    {

    }

    Double time() const { return (1.0 / 44100.0) * samplePos_; }

    void stop() { stop_ = true; wait(); }

    void run()
    {
        samplePos_ = 0;
        while (!stop_)
        {
            scene_->calculateAudioBlock(samplePos_, 1);
            samplePos_ += scene_->bufferSize(1);

        #ifndef NDEBUG
            // leave some room when in debug mode
            usleep(1000);
        #endif
        }
    }

private:
    Scene * scene_;
    volatile bool stop_;
    SamplePos samplePos_;
};












MainWindow::MainWindow(QWidget *parent) :
    QMainWindow     (parent),
    scene_          (0),
    objectTreeModel_(0),
    seqFloatView_   (0),
    qobjectView_    (0),
    testThread_     (0),

    currentSceneDirectory_(IO::Files::directory(IO::FT_SCENE)),
    statusMessageTimeout_(1000 * 5)
{
    setObjectName("_MainWindow");

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(QIcon(":/icon/mo.png"));

    createMainMenu_();
    createWidgets_();
    createObjects_();

    // read previous geometry
    if (!restoreAllGeometry_())
    {
        // center window
        QRect r = application->desktop()->screenGeometry(pos());
        setGeometry((r.width() - width())/2,
                    (r.height() - height())/2,
                     width(), height());
    }

    updateWindowTitle_();
}

MainWindow::~MainWindow()
{
    if (testThread_)
        testThread_->stop();
}

void MainWindow::createWidgets_()
{
    setCentralWidget(new QWidget(this));
    centralWidget()->setObjectName("_centralwidget");

    // scene settings class
    sceneSettings_ = new SceneSettings(this);

    // status bar
    setStatusBar(new QStatusBar(this));
    statusBar()->addPermanentWidget(sysInfoLabel_ = new QLabel(statusBar()));

    // main layout
    auto l0 = new QHBoxLayout(centralWidget());
    l0->setMargin(0);
    l0->setSpacing(1);

        auto leftContainer = new QWidget(this);
        l0->addWidget(leftContainer);
        leftContainer->setObjectName("_left_container");
        leftContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        leftContainer->setMinimumWidth(270);

        auto lv = new QVBoxLayout(leftContainer);
        lv->setMargin(0);
        //lv->setSizeConstraint(QLayout::SetMinAndMaxSize);

            // object tree view
            objectTreeView_ = new ObjectTreeView(this);
            lv->addWidget(objectTreeView_);
            objectTreeView_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            //objectTreeView_->setMinimumWidth(240);
            //objectTreeView_->setMaximumWidth(450);
            connect(objectTreeView_, SIGNAL(editActionsChanged(const QObject*,QList<QAction*>)),
                    SLOT(setEditActions_(const QObject*,QList<QAction*>)));
            connect(objectTreeView_, SIGNAL(objectSelected(MO::Object*)),
                    SLOT(objectSelected_(MO::Object*)));

            // object tree model
            objectTreeModel_ = new ObjectTreeModel(0, this);
            connect(objectTreeModel_, SIGNAL(sceneChanged()),
                    this, SLOT(sceneChanged_()));
            objectTreeView_->setObjectModel(objectTreeModel_);

            // object editor
            objectView_ = new ObjectView(this);
            lv->addWidget(objectView_);
            objectView_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            //objectView_->setMinimumWidth(240);
            connect(objectView_, SIGNAL(objectSelected(MO::Object*)),
                                        this, SLOT(objectSelected_(MO::Object*)));
            connect(objectView_, SIGNAL(statusTipChanged(QString)),
                    statusBar(), SLOT(showMessage(QString)));
        //l0->setStretchFactor(lv, -1);

        lv = new QVBoxLayout();
        l0->addLayout(lv);

            spacer_ = new Spacer(Qt::Vertical, this);
            lv->addWidget(spacer_);

        lv = new QVBoxLayout();
        l0->addLayout(lv);

            // sequencer
            sequencer_ = new Sequencer(this);
            sequencer_->setSceneSettings(sceneSettings_);
            sequencer_->setMinimumHeight(300);
            lv->addWidget(sequencer_);
            connect(sequencer_, &Sequencer::sequenceSelected, [this](Sequence * seq)
            {
                objectSelected_(seq);
            });

            //spacer2_ = new Spacer(Qt::Horizontal, this);
            //lv->addWidget(spacer2_);

            // SequenceFloat view
            seqFloatView_ = new SequenceFloatView(this);
            seqFloatView_->setVisible(false);
            seqFloatView_->setSceneSettings(sceneSettings_);
            lv->addWidget(seqFloatView_);
            connect(seqFloatView_, SIGNAL(statusTipChanged(QString)),
                    statusBar(), SLOT(showMessage(QString)));


    spacer_->setWidgets(leftContainer, sequencer_);
    //spacer2_->setWidgets(sequencer_, seqFloatView_);



    setMinimumSize(800,500);

}


void MainWindow::createMainMenu_()
{
    QMenu * m;
    QAction * a;

    // ######### FILE MENU #########
    m = editMenu_ = new QMenu(tr("File"), menuBar());
    menuBar()->addMenu(m);

    m->addAction(a = new QAction(tr("New scene"), menuBar()));
    connect(a, SIGNAL(triggered()), this, SLOT(newScene()));

    m->addSeparator();

    m->addAction(a = new QAction(tr("Load scene"), menuBar()));
    connect(a, SIGNAL(triggered()), this, SLOT(loadScene()));

    m->addAction(a = actionSaveScene_ = new QAction(tr("Save scene"), menuBar()));
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(saveScene()));

    m->addAction(a = new QAction(tr("Save scene as ..."), menuBar()));
    a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(saveSceneAs()));


    // ######### EDIT MENU #########
    m = editMenu_ = new QMenu(tr("Edit"), menuBar());
    menuBar()->addMenu(m);
    // will be updated from child widgets


    // ######### RENDER MENU #########
    m = new QMenu(tr("Render"), menuBar());
    menuBar()->addMenu(m);

    QActionGroup * ag = new QActionGroup(menuBar());
    m->addAction(a = new QAction(tr("Start"), menuBar()));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F7);
    a->setCheckable(true);
    connect(a, SIGNAL(triggered()), this, SLOT(start()));

    m->addAction(a = new QAction(tr("Stop"), menuBar()));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F8);
    a->setCheckable(true);
    a->setChecked(true);
    connect(a, SIGNAL(triggered()), this, SLOT(stop()));

    m->addAction(a = new QAction(tr("Render to disk"), menuBar()));
    ag->addAction(a);
    connect(a, SIGNAL(triggered()), this, SLOT(renderToDisk()));

    // ######### OPTIONS MENU #########
    m = new QMenu(tr("Options"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("Audio settings"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            scene_->closeAudio();
            AudioDialog diag;
            diag.exec();
        });


    // ######### DEBUG MENU #########
    m = new QMenu(tr("Debug"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("QObject inspector"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            if (!qobjectView_)
                qobjectView_ = new QObjectInspector(this, this);
            qobjectView_->setRootObject(this);
            qobjectView_->show();
        });

        m->addAction(a = new QAction(tr("Test transformation speed"), m));
        connect(a, SIGNAL(triggered()), SLOT(testSceneTransform_()));

        m->addAction(a = new QAction(tr("Create Debug Scene"), m));
        connect(a, SIGNAL(triggered()), SLOT(createDebugScene_()));

        m->addAction(a = new QAction(tr("Reset ObjectTreeModel"), m));
        connect(a, SIGNAL(triggered()), SLOT(resetTreeModel_()));

        m->addAction(a = new QAction(tr("Run test thread"), m));
        a->setCheckable(true);
        connect(a, SIGNAL(triggered()), SLOT(runTestThread_()));

        a = new QAction(tr("Geometry dialog"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            GeometryDialog * diag = new GeometryDialog(0, this);
            connect(diag, SIGNAL(finished(int)), diag, SLOT(deleteLater()));
            diag->show();
        });

        a = new QAction(tr("Projector dialog"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto pv = new ProjectorSetupWidget();
            pv->setAttribute(Qt::WA_DeleteOnClose, true);
            pv->show();
        });

}

void MainWindow::setSceneObject(Scene * s, const SceneSettings * set)
{
    MO_DEBUG_GUI("MainWindow::setSceneObject(" << s << ")");

    MO_ASSERT(s, "MainWindow::setSceneObject() with NULL scene");
    MO_ASSERT(s != scene_, "MainWindow::setSceneObject() with same scene");

    if (scene_)
    {
        scene_->kill();
        scene_->deleteLater();
    }

    scene_ = s;

    // manage memory
    scene_->setParent(this);

    // clear or init scene widget settings
    if (!set)
        sceneSettings_->clear();
    else
        *sceneSettings_ = *set;

    MO_ASSERT(glManager_ && glWindow_, "");

    // connect to render window
    connect(glManager_, SIGNAL(renderRequest(uint)), scene_, SLOT(renderScene(uint)));
    connect(glManager_, SIGNAL(contextCreated(uint,MO::GL::Context*)),
                scene_, SLOT(setGlContext(uint,MO::GL::Context*)));

    connect(scene_, SIGNAL(renderRequest()), glWindow_, SLOT(renderLater()));

    if (glWindow_->context())
        scene_->setGlContext(glWindow_->threadId(), glWindow_->context());
    connect(scene_, SIGNAL(playbackStarted()),
            glWindow_, SLOT(startAnimation()));
    connect(scene_, SIGNAL(playbackStopped()),
            glWindow_, SLOT(stopAnimation()));

    connect(seqFloatView_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));
    connect(sequencer_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));

    // scene changes
    connect(scene_, SIGNAL(objectAdded(MO::Object*)), this, SLOT(treeChanged_()));
    connect(scene_, SIGNAL(objectDeleted(MO::Object*)), this, SLOT(treeChanged_()));
    connect(scene_, SIGNAL(childrenSwapped(MO::Object*,int,int)), this, SLOT(treeChanged_()));
    connect(scene_, SIGNAL(sequenceChanged(MO::Sequence*)), this, SLOT(sceneChanged_()));
    connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)), this, SLOT(sceneChanged_()));
    connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)),
            objectTreeView_, SLOT(columnMoved()/* force update */));

    // update widgets

    scene_->setObjectModel(objectTreeModel_);
    connect(scene_, SIGNAL(sceneTimeChanged(Double)),
            seqFloatView_, SLOT(setSceneTime(Double)));
    connect(scene_, SIGNAL(sceneTimeChanged(Double)),
            sequencer_, SLOT(setSceneTime(Double)));

    objectView_->setObject(0);

    seqFloatView_->setScene(scene_);
    seqFloatView_->setSequence(0);

    sequencer_->setTracks(scene_);

    glWindow_->renderLater();

    updateSystemInfo_();
}


void MainWindow::createObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);
    glWindow_->show();

    try
    {
        QString fn = settings->getValue("File/scene").toString();
        if (!fn.isEmpty())
            loadScene_(fn);
        else
            newScene();
        //loadScene_("./lookat.mo3");
        //loadScene_("./dreh.mo3");
    }
    catch (IoException& e)
    {
        MO_WARNING(e.what());
        newScene();
    }

    // sysinfo at some interval
    sysInfoTimer_ = new QTimer(this);
    sysInfoTimer_->setInterval(5000);
    connect(sysInfoTimer_, SIGNAL(timeout()), this, SLOT(updateSystemInfo_()));
    sysInfoTimer_->start();

    updateSystemInfo_();
}

void MainWindow::resetTreeModel_()
{
    objectTreeModel_->setSceneObject(scene_);
}

void MainWindow::createDebugScene_()
{
    /*
    auto scene = ObjectFactory::createSceneObject();

    auto cam = scene->addObject(scene, ObjectFactory::createObject("Camera"));
        cam->addObject(ObjectFactory::createObject("Translation"));
        cam->addObject(ObjectFactory::createObject("AxisRotation"));
        cam->addObject(ObjectFactory::createObject("SequenceFloat"));
        cam->addObject(ObjectFactory::createObject("SequenceFloat"));
        for (int i=0; i<21; ++i)
        {
            auto mic = cam->addObject(ObjectFactory::createObject("Microphone"));
                mic->addObject(ObjectFactory::createObject("AxisRotation"));
                mic->addObject(ObjectFactory::createObject("Translation"));
        }

    for (int i=0; i<20; ++i)
    {
        auto model = scene->addObject(ObjectFactory::createObject("Model3d"));
            model->addObject(ObjectFactory::createObject("Translation"));
            model->addObject(ObjectFactory::createObject("AxisRotation"));
            auto snd = model->addObject(ObjectFactory::createObject("SoundSource"));
                snd->addObject(ObjectFactory::createObject("AxisRotation"));
                snd->addObject(ObjectFactory::createObject("Translation"));
    }

    setSceneObject(scene);
    */
}



void MainWindow::closeEvent(QCloseEvent * e)
{
    if (okayToChangeScene_())
    {
        e->accept();

        saveAllGeometry_();

        if (scene_)
            scene_->kill();
    }
    else
        e->ignore();
}

void MainWindow::saveAllGeometry_()
{
    settings->saveGeometry(this);
    if (glWindow_)
        settings->saveGeometry(glWindow_);
}

bool MainWindow::restoreAllGeometry_()
{
    bool r = settings->restoreGeometry(this);
    if (glWindow_)
        settings->restoreGeometry(glWindow_);
    return r;
}


void MainWindow::objectSelected_(Object * o)
{
    // update object editor
    objectView_->setObject(o);

    // update sequencer
    // XXX
    //sequencer_->setTracks(scene_);

    // update sequence editor
    if (o && o->type() == Object::T_SEQUENCE_FLOAT)
    {
        seqFloatView_->setSequence(static_cast<SequenceFloat*>(o));
        if (!seqFloatView_->isVisible())
            seqFloatView_->setVisible(true);
    }
    else
        seqFloatView_->setVisible(false);

    // update tree view
    objectTreeView_->setFocusIndex(o);
}

void MainWindow::treeChanged_()
{
    sequencer_->setTracks(scene_);
}

void MainWindow::sceneChanged_()
{
    if (!sceneNotSaved_)
    {
        sceneNotSaved_ = true;
        updateWindowTitle_();
    }
}

void MainWindow::setEditActions_(const QObject *, QList<QAction *> actions)
{
    editMenu_->clear();
    editMenu_->addActions(actions);
}

void MainWindow::testSceneTransform_()
{
    QTime t;

    int num = 10000000;
    int i = 0;
    int e = 0;

    t.start();
    for (; i < num && e <= 1000; ++i)
    {
        for (int j=0; j<1000; ++j, ++i)
            scene_->calculateSceneTransform(0, 0, 0.01 * i);

        e = t.elapsed();
    }
    num = i;
    const Double elapsed = (Double)e / 1000.0;

    QMessageBox::information(this, tr("Scene transformation test"),
        tr("Calculating %1 frames of transformation\n"
           "which took %2 seconds.\n"
           "This is %3 milli-seconds per frame\n"
           "and %4 frames per second.")
                             .arg(num)
                             .arg(elapsed)
                             .arg((elapsed*1000)/num)
                             .arg((int)((Double)num/elapsed))
           );
}

void MainWindow::runTestThread_()
{
    if (!testThread_)
    {
        testThread_ = new TestThread(scene_, this);
        testThread_->start();
    }
    else
    {
        testThread_->stop();
        testThread_->deleteLater();
        testThread_ = 0;
    }
}

void MainWindow::updateSystemInfo_()
{
    QString info = tr("%1mb/%2mb")
            .arg(Memory::allocated()/1024/1024)
            .arg(GL::Texture::memoryAll()/1024/1024);

    if (testThread_)
    {
        static Double lastTime = 0.0;
        Double
            time = testThread_->time(),
            delta = time - lastTime,
            realtime = delta / (0.001*sysInfoTimer_->interval());
        lastTime = time;

        info.append(" " + tr("%1s %2x").arg(time).arg(realtime));
    }

    sysInfoLabel_->setText(info);
}

void MainWindow::updateWindowTitle_()
{
    QString t;

    if (sceneNotSaved_)
        t = "* ";

    if (!currentSceneFilename_.isEmpty())
    {
        t += QFileInfo(currentSceneFilename_).fileName() + " - ";
    }

    setWindowTitle(t + windowTitleString);
}

void MainWindow::updateWidgetsActivity_()
{
    actionSaveScene_->setEnabled( !currentSceneFilename_.isEmpty() );
}

void MainWindow::start()
{
    scene_->start();
}

void MainWindow::stop()
{
    scene_->stop();
}

/*
void MainWindow::openAudio_()
{
    if (audioDevice_->isAudioConfigured())
        audioDevice_->initFromSettings();

    audioDevice_->setCallback([=](const F32*, F32 * out)
    {
//        scene_->calculateAudioBlock();
    });
}

void MainWindow::closeAudio_()
{
    audioDevice_->close();
}

void MainWindow::startAudio_()
{
    audioDevice_->start();
}

void MainWindow::stopAudio_()
{
    audioDevice_->stop();
}
*/



void MainWindow::newScene()
{
    if (!okayToChangeScene_())
        return;

    setSceneObject( ObjectFactory::createSceneObject() );
    currentSceneFilename_.clear();
    IO::Files::setFilename(IO::FT_SCENE, "");
    sceneNotSaved_ = false;
    updateWindowTitle_();
    updateWidgetsActivity_();
}

bool MainWindow::okayToChangeScene_()
{
    if (!sceneNotSaved_)
        return true;

    QMessageBox::StandardButton res =
    QMessageBox::question(this, tr("Project not saved"),
                          tr("The current project has not been saved\n"
                             "Do you want to save it?\n(%1)")
                          .arg(currentSceneFilename_.isEmpty()?
                                   tr("choose filename")
                                 : currentSceneFilename_)
                          , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                          QMessageBox::Yes);

    if (res == QMessageBox::Cancel)
        return false;

    if (res == QMessageBox::No)
        return true;

    return saveScene();
}

QString MainWindow::getSceneSaveFilename_()
{
    while (true)
    {
        QString fn = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Scene"),
                    currentSceneDirectory_,
                    "MatrixOptimizer .mo3 (*.mo3)",
                    0,
                    QFileDialog::DontConfirmOverwrite);

        // cancelled
        if (fn.isEmpty())
            return QString();

        // remember the directory
        currentSceneDirectory_ = QDir(fn).absolutePath();

        // complete filename
        if (!fn.endsWith(".mo3"))
            fn.append(".mo3");

        // check existence
        if (!QFile::exists(fn))
            return fn;

        // ask for overwrite
        QMessageBox::StandardButton res =
        QMessageBox::question(this, tr("File already exists"),
                              tr("The file %1 already exists.\n"
                                 "Do you want to replace it?").arg(fn)
                              , QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              QMessageBox::No);

        if (res == QMessageBox::Yes)
            return fn;

        if (res == QMessageBox::Cancel)
            return QString();
    }
}

bool MainWindow::saveScene()
{
    if (!scene_)
        return false;

    QString fn = currentSceneFilename_;

    if (fn.isEmpty())
        fn = getSceneSaveFilename_();

    return saveScene_(fn);
}

void MainWindow::saveSceneAs()
{
    if (!scene_)
        return;

    QString fn = getSceneSaveFilename_();

    saveScene_(fn);
}


void MainWindow::loadScene()
{
    if (!okayToChangeScene_())
        return;

    QString fn = QFileDialog::getOpenFileName(
                this,
                tr("Load Scene"),
                currentSceneDirectory_,
                "MatrixOptimizer mo3 (*.mo3)");

    loadScene_(fn);
}

void MainWindow::loadScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        // read scene file
        Scene * scene;
        try
        {
            scene = ObjectFactory::loadScene(fn);
        }
        catch (Exception&)
        {
            statusBar()->showMessage(tr("Could not open project '%1'").arg(fn));
            return;
        }

        // read gui settings
        SceneSettings sceneSettings;
        try
        {
            QString guifn = sceneSettings.getSettingsFileName(fn);
            if (QFileInfo(guifn).exists())
            {
                sceneSettings.loadFile(guifn);
            }
            else
                MO_DEBUG_IO("No scene-settings " << guifn);
        }
        catch (Exception & e) { }

        setSceneObject(scene, &sceneSettings);

        statusBar()->showMessage(tr("Opened %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        currentSceneDirectory_ = QFileInfo(fn).absolutePath();
        IO::Files::setFilename(IO::FT_SCENE, currentSceneFilename_);
        IO::Files::setDirectory(IO::FT_SCENE, currentSceneDirectory_);
        sceneNotSaved_ = false;
        updateWindowTitle_();
        updateWidgetsActivity_();
    }
    else
        statusBar()->showMessage(tr("loading cancelled"), statusMessageTimeout_);
}

bool MainWindow::saveScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        try
        {
            // actually save the scene
            ObjectFactory::saveScene(fn, scene_);
            // save the gui settings for the scene
            sceneSettings_->saveFile(sceneSettings_->getSettingsFileName(fn));
        }
        catch (Exception&)
        {
            statusBar()->showMessage("SAVING FAILED!");
            return false;
        }

        statusBar()->showMessage(tr("Saved %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        currentSceneDirectory_ = QFileInfo(fn).absolutePath();
        IO::Files::setFilename(IO::FT_SCENE, currentSceneFilename_);
        IO::Files::setDirectory(IO::FT_SCENE, currentSceneDirectory_);
        sceneNotSaved_ = false;
        updateWindowTitle_();
        updateWidgetsActivity_();
        return true;
    }

    statusBar()->showMessage(tr("saving cancelled"), statusMessageTimeout_);
    return false;
}


void MainWindow::renderToDisk()
{
    auto ren = new Renderer(this);

    ren->setScene(scene_);
    ren->setOutputPath("/home/defgsus/prog/qt_project/mo/matrixoptimizer/render");

    if (!ren->prepareRendering())
    {
        QMessageBox::critical(this, tr("Render to disk"),
                              tr("Sorry, but rendering to disk failed"));
        ren->deleteLater();
        return;
    }

    connect(ren, SIGNAL(finished()), ren, SLOT(deleteLater()));

    ren->start();
}


} // namespace GUI
} // namespace MO
