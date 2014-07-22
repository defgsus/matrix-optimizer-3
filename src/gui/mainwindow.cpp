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
#include "model/objecttreemodel.h"
#include "io/datastream.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "io/error.h"
#include "io/memory.h"
#include "io/settings.h"

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
        blockLength_ = 128;
        while (!stop_)
        {
            scene_->calculateAudioBlock(samplePos_, blockLength_, 1);
            samplePos_ += blockLength_;

        #ifndef NDEBUG
            // leave some room when in debug mode
            usleep(1000);
        #endif
        }
    }

private:
    Scene * scene_;
    volatile bool stop_;
    SamplePos samplePos_, blockLength_;
};












MainWindow::MainWindow(QWidget *parent) :
    QMainWindow     (parent),
    scene_          (0),
    objectTreeModel_(0),
    seqFloatView_   (0),
    qobjectView_    (0),
    testThread_     (0),

    currentSceneDirectory_(settings->getValue("Directory/scene").toString()),
    statusMessageTimeout_(1000)
{

    setAttribute(Qt::WA_DeleteOnClose, true);

    createMainMenu_();
    createWidgets_();
    createObjects_();

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

    // ------------ connections --------------





    // --------- io ----------
/*
    QMenu * m;
    QAction * a;

    m = new QMenu(tr("Timeline"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("save timeline"), m);
        m->addAction(a);
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, &QAction::triggered, [=]()
        {
            tl->saveFile("./timeline.bin");
        });

        a = new QAction(tr("load timeline"), m);
        m->addAction(a);
        a->setShortcut(Qt::CTRL + Qt::Key_L);
        connect(a, &QAction::triggered, [=]()
        {
            tl->loadFile("./timeline.bin");
            tlv->unselect();
        });
*/
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
}

void MainWindow::setSceneObject(Scene * s)
{
    MO_ASSERT(s, "MainWindow::setSceneObject() with NULL scene");
    MO_ASSERT(s != scene_, "MainWindow::setSceneObject() with same scene");

    if (scene_)
        scene_->deleteLater();

    scene_ = s;

    // manage memory
    scene_->setParent(this);

    // clear scene widget settings
    sceneSettings_->clear();

    MO_ASSERT(glManager_ && glWindow_, "");

    // connect to render window
    connect(glManager_, SIGNAL(renderRequest()), scene_, SLOT(renderScene()));
    connect(glManager_, SIGNAL(contextCreated(MO::GL::Context*)),
                scene_, SLOT(setGlContext(MO::GL::Context*)));

    connect(scene_, SIGNAL(renderRequest()), glWindow_, SLOT(renderLater()));

    if (glWindow_->context())
        scene_->setGlContext(glWindow_->context());

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
    glWindow_ = glManager_->createGlWindow();
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
    }
    else
        e->ignore();
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
            scene_->calculateSceneTransform(0, 0.01 * i);

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
    QString info = tr("%1mb").arg(Memory::allocated()/1024/1024);

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

void MainWindow::newScene()
{
    if (!okayToChangeScene_())
        return;

    setSceneObject( ObjectFactory::createSceneObject() );
    currentSceneFilename_.clear();
    settings->setValue("File/scene", "");
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

    if (!fn.isEmpty())
    {
        currentSceneDirectory_ = QDir(fn).absolutePath();
        settings->setValue("Directory/scene", currentSceneDirectory_);
    }

    loadScene_(fn);
}

void MainWindow::loadScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        setSceneObject(ObjectFactory::loadScene(fn));

        statusBar()->showMessage(tr("Opened %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        settings->setValue("File/scene", currentSceneFilename_);
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
        ObjectFactory::saveScene(fn, scene_);

        statusBar()->showMessage(tr("Saved %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        settings->setValue("File/scene", currentSceneFilename_);
        sceneNotSaved_ = false;
        updateWindowTitle_();
        updateWidgetsActivity_();
        return true;
    }

    statusBar()->showMessage(tr("saving cancelled"), statusMessageTimeout_);
    return false;
}

} // namespace GUI
} // namespace MO
