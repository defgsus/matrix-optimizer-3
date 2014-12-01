/** @file mainwidgetcontroller.cpp

    @brief Controller for all main edit widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 17.10.2014</p>
*/

#include <QLayout>
#include <QStatusBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QStatusBar>
#include <QTimer>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QKeyEvent>
#include <QTime>
#include <QActionGroup>

#include "mainwidgetcontroller.h"
#include "io/error.h"
#include "io/log.h"
#include "io/settings.h"
#include "io/application.h"
#include "io/memory.h"
#include "io/version.h"
#include "io/filemanager.h"
#include "io/helpexporterhtml.h"
#include "gui/timeline1dview.h"
#include "gui/timeline1drulerview.h"
#include "gui/ruler.h"
#include "gui/qobjectinspector.h"
#include "gui/objecttreeview.h"
#include "gui/objectview.h"
#include "gui/sequenceview.h"
#include "gui/sequencer.h"
#include "gui/clipview.h"
#include "gui/helpdialog.h"
#include "gui/audiodialog.h"
#include "gui/geometrydialog.h"
#include "gui/equationdisplaydialog.h"
#include "gui/audiolinkwindow.h"
#include "gui/sceneconvertdialog.h"
#include "gui/projectorsetupdialog.h"
#include "gui/networkdialog.h"
#include "gui/midisettingsdialog.h"
#include "gui/infowindow.h"
#include "gui/timelineeditdialog.h"
#include "gui/audiofilterdialog.h"
#include "gui/serverdialog.h"
#include "gui/resolutiondialog.h"
#include "gui/objectgraphview.h"
#include "gui/widget/envelopewidget.h"
#include "gui/widget/transportwidget.h"
#include "gui/widget/spacer.h"
#include "gui/util/scenesettings.h"
#include "model/objecttreemodel.h"
#include "io/datastream.h"
#include "io/files.h"
#include "io/povrayexporter.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "gl/texture.h"
#include "engine/renderer.h"
#include "engine/serverengine.h"
#include "object/objectfactory.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/sequencefloat.h"
#include "object/clipcontainer.h"
#include "object/util/objectmodulatorgraph.h"
#include "object/util/objecteditor.h"
#include "object/audio/objectdsppath.h"
#include "model/treemodel.h"
#include "object/util/objecttree.h"
#include "tool/commonresolutions.h"
#include "engine/serverengine.h"

namespace MO {
namespace GUI {




class TestThread : public QThread
{
public:
    TestThread(Scene * scene, bool newVersion, QObject * parent)
        :   QThread(parent),
          scene_  (scene),
          stop_   (false),
          new_    (newVersion)
    {

    }

    Double time() const { return (1.0 / 44100.0) * samplePos_; }

    void stop() { stop_ = true; wait(); }

    void run()
    {
        setCurrentThreadName("AUDIOTEST");

        ObjectDspPath dsp;
        dsp.createPath(scene_, 44100, 2048);

        samplePos_ = 0;
        while (!stop_)
        {
            if (new_)
            {
                dsp.calcTransformations(samplePos_, MO_AUDIO_THREAD);
                samplePos_ += dsp.bufferSize();
            }
            else
            {
                scene_->calculateAudioBlock(samplePos_, MO_AUDIO_THREAD);
                samplePos_ += scene_->bufferSize(MO_AUDIO_THREAD);
            }

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
    bool new_;
};



























MainWidgetController::MainWidgetController(QMainWindow * win)
    : QObject           (win),
      window_           (win),
      scene_            (0),
#ifndef MO_DISABLE_TREE
      objectTreeModel_  (0),
#endif
      outputSize_       (512, 512),
      glManager_        (0),
      glWindow_         (0),
      objectEditor_     (0),
      objectView_       (0),
      sceneSettings_    (0),
      sequencer_        (0),
      clipView_         (0),
      seqView_          (0),
      transportWidget_  (0),
      qobjectInspector_ (0),
      serverDialog_     (0),
      statusBar_        (0),
      sysInfoLabel_     (0),
      testThread_       (0),
      sceneNotSaved_    (false),
      statusMessageTimeout_(7 * 1000)
{
    aResolutionOutput_ = 0;
    setObjectName("_MainWidgetController");

    createObjects_();

    // start server automatically
    if (settings->value("Server/running").toBool()
        && !serverEngine().isRunning())
            serverEngine().open();
}

MainWidgetController::~MainWidgetController()
{
    if (testThread_)
        testThread_->stop();
}

void MainWidgetController::createObjects_()
{
    // scene settings class
    sceneSettings_ = new SceneSettings(this);

    // editor
    objectEditor_ = new ObjectEditor(this);
    connect(objectEditor_, SIGNAL(objectNameChanged(MO::Object*)), this, SLOT(onObjectNameChanged_(MO::Object*)));
    connect(objectEditor_, SIGNAL(objectAdded(MO::Object*)), this, SLOT(onObjectAdded_(MO::Object*)));
    connect(objectEditor_, SIGNAL(objectDeleted(const MO::Object*)), this, SLOT(onObjectDeleted_(const MO::Object*)));
    connect(objectEditor_, SIGNAL(childrenSwapped(MO::Object*,int,int)), this, SLOT(onTreeChanged_()));
    connect(objectEditor_, SIGNAL(sequenceChanged(MO::Sequence*)), this, SLOT(onSceneChanged_()));
    connect(objectEditor_, SIGNAL(parameterChanged(MO::Parameter*)), this, SLOT(onSceneChanged_()));

    // status bar
    statusBar_ = new QStatusBar(window_);
    statusBar_->addPermanentWidget(sysInfoLabel_ = new QLabel(statusBar()));

    // transport widget
    transportWidget_ = new TransportWidget(window_);


#ifndef MO_DISABLE_TREE
    // object tree view
    objectTreeView_ = new ObjectTreeView(window_);
    objectTreeView_->setSceneSettings(sceneSettings_);
    connect(objectTreeView_, SIGNAL(editActionsChanged(const QObject*,QList<QAction*>)),
            SLOT(setEditActions_(const QObject*,QList<QAction*>)));
    connect(objectTreeView_, SIGNAL(objectSelected(MO::Object*)),
            SLOT(onObjectSelectedTree_(MO::Object*)));
#endif

    // object graph view
    objectGraphView_ = new ObjectGraphView(window_);
    connect(objectGraphView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedGraphView_(MO::Object*)));

#ifndef MO_DISABLE_TREE
    // object tree model
    objectTreeModel_ = new ObjectTreeModel(0, this);
    //connect(objectTreeModel_, SIGNAL(sceneChanged()),
    //        this, SLOT(onSceneChanged_()));
    objectTreeView_->setObjectModel(objectTreeModel_);
    objectTreeModel_->setSceneSettings(sceneSettings_);
#endif

    // object View
    objectView_ = new ObjectView(window_);
    objectView_->setSceneSettings(sceneSettings_);
    connect(objectView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedObjectView_(MO::Object*)));
    connect(objectView_, SIGNAL(statusTipChanged(QString)),
            statusBar_, SLOT(showMessage(QString)));

    // sequencer
    sequencer_ = new Sequencer(window_);
    sequencer_->setSceneSettings(sceneSettings_);
    sequencer_->setMinimumHeight(120);
    connect(sequencer_, SIGNAL(sequenceSelected(MO::Sequence*)),
            this, SLOT(onObjectSelectedSequencer_(MO::Sequence*)));

    // clipview
    clipView_ = new ClipView(window_);
    clipView_->setVisible(false);
    connect(clipView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedClipView_(MO::Object*)));
    connect(clipView_, SIGNAL(clipsMoved()),
            this, SLOT(onSceneChanged_()));

    // SequenceFloat view
    seqView_ = new SequenceView(window_);
    seqView_->setVisible(false);
    seqView_->setSceneSettings(sceneSettings_);
    connect(seqView_, SIGNAL(statusTipChanged(QString)),
            statusBar_, SLOT(showMessage(QString)));
    connect(seqView_, SIGNAL(clicked()),
            this, SLOT(onSequenceClicked_()));

    // gl manager and window
    glManager_ = new GL::Manager(this);
    connect(glManager_, SIGNAL(outputSizeChanged(QSize)),
            this, SLOT(onOutputSizeChanged_(QSize)));

    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);

    connect(glWindow_, SIGNAL(keyPressed(QKeyEvent*)),
            this, SLOT(onWindowKeyPressed_(QKeyEvent*)));

    glWindow_->show();

    // sysinfo at some interval
    sysInfoTimer_ = new QTimer(this);
    sysInfoTimer_->setInterval(5000);
    connect(sysInfoTimer_, SIGNAL(timeout()), this, SLOT(updateSystemInfo_()));
    sysInfoTimer_->start();

    updateSystemInfo_();
}


void MainWidgetController::createMainMenu(QMenuBar * menuBar)
{
    QMenu * m;
    QAction * a;

    // ######### FILE MENU #########
    m = new QMenu(tr("File"), menuBar);
    menuBar->addMenu(m);

    m->addAction(a = new QAction(tr("New scene"), menuBar));
    connect(a, SIGNAL(triggered()), this, SLOT(newScene()));

    m->addSeparator();

    m->addAction(a = new QAction(tr("Load scene"), menuBar));
    a->setShortcut(Qt::CTRL + Qt::Key_L);
    connect(a, SIGNAL(triggered()), this, SLOT(loadScene()));

    m->addAction(a = actionSaveScene_ = new QAction(tr("Save scene"), menuBar));
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(saveScene()));

    m->addAction(a = new QAction(tr("Save scene as ..."), menuBar));
    a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(saveSceneAs()));

    // ######### EDIT MENU #########
    m = menuEdit_ = new QMenu(tr("Edit"), menuBar);
    menuBar->addMenu(m);
    // will be updated from child widgets


    // ######### RENDER MENU #########
    m = new QMenu(tr("Render"), menuBar);
    menuBar->addMenu(m);

    QActionGroup * ag = new QActionGroup(menuBar);
    m->addAction(a = new QAction(tr("Start"), menuBar));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F7);
    a->setCheckable(true);
    connect(a, SIGNAL(triggered()), this, SLOT(start()));

    m->addAction(a = new QAction(tr("Stop"), menuBar));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F8);
    a->setCheckable(true);
    a->setChecked(true);
    connect(a, SIGNAL(triggered()), this, SLOT(stop()));

    m->addSeparator();

    // ##### RESOLUTION SUBMENU #####
    auto sub = new QMenu(tr("Resolution"), menuBar);
    m->addMenu(sub);

        ag = new QActionGroup(sub);
        sub->addAction( a = aResolutionOutput_ = new QAction(tr("Same as output window"), sub) );
        a->setCheckable(true);
        connect(a, SIGNAL(triggered()), this, SLOT(onResolutionOutput_()));
        ag->addAction(a);

        sub->addAction( a = aResolutionCustom_ = new QAction(tr("Custom ... "), sub) );
        a->setCheckable(true);
        connect(a, SIGNAL(triggered()), this, SLOT(onResolutionCustom_()));
        ag->addAction(a);

        auto sub2 = menuResolutions_ = new QMenu(tr("Predefined"), sub);
        a = aResolutionPredefined_ = sub->addMenu(sub2);
        a->setCheckable(true);
        ag->addAction(a);

            CommonResolutions::addResolutionActions(sub2, true);
            connect(sub2, SIGNAL(triggered(QAction*)),
                    this, SLOT(onResolutionPredefined_(QAction*)));


    m->addSeparator();

        // ##### DEBUG VISIBILITY SUBMENU #####
        sub = new QMenu(tr("Visibility"), menuBar);
        m->addMenu(sub);

        sub->addAction(a = aDrawCameras_ = new QAction(tr("Show cameras"), sub));
        a->setCheckable(true);
        a->setShortcut(Qt::ALT + Qt::Key_1);
        connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

        sub->addAction(a = aDrawLightSources_= new QAction(tr("Show lights"), sub));
        a->setCheckable(true);
        a->setShortcut(Qt::ALT + Qt::Key_2);
        connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

        sub->addAction(a = aDrawMicrophones_ = new QAction(tr("Show microphones"), sub));
        a->setCheckable(true);
        a->setShortcut(Qt::ALT + Qt::Key_3);
        connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

        sub->addAction(a = aDrawAudioSources_ = new QAction(tr("Show soundsources"), sub));
        a->setCheckable(true);
        a->setShortcut(Qt::ALT + Qt::Key_4);
        connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

    m->addSeparator();

        // ##### PROJECTOR INDEX SUBMENU #####
        sub = menuProjectorIndex_ = new QMenu(tr("Projector index"), menuBar);
        m->addMenu(sub);
        // will be updated by onProjectionSettingsChanged_();

        connect(sub, &QMenu::triggered, [=](QAction*a)
        {
            settings->setClientIndex(a->data().toInt());
            updateSceneProjectionSettings_();
        });


    m->addSeparator();

    m->addAction(a = new QAction(tr("Render to disk"), menuBar));
    ag->addAction(a);
    connect(a, SIGNAL(triggered()), this, SLOT(renderToDisk()));

    // ######### OPTIONS MENU #########
    m = new QMenu(tr("Options"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("Audio settings"), m);
        m->addAction(a);
        a->setIcon(QIcon(":/icon/obj_soundsource.png"));
        connect(a, &QAction::triggered, [=]()
        {
            scene_->closeAudio();
            AudioDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Midi settings"), m);
        m->addAction(a);
        a->setIcon(QIcon(":/icon/midi.png"));
        connect(a, &QAction::triggered, [=]()
        {
            scene_->closeAudio();
            MidiSettingsDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Server/client settings"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            if (!serverDialog_)
            {
                serverDialog_ = new ServerDialog(window_);
                connect(serverDialog_, SIGNAL(sendScene()),
                        this, SLOT(sendSceneToClients_()));
            }
            if (serverDialog_->isHidden())
                serverDialog_->show();
        });

        a = new QAction(tr("Network settings"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            NetworkDialog diag(window_);
            diag.exec();
        });

        a = new QAction(tr("Projector setup"), m);
        m->addAction(a);
        a->setIcon(QIcon(":/icon/obj_camera.png"));
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new ProjectorSetupDialog(window_);
            diag->setAttribute(Qt::WA_DeleteOnClose, true);
            connect(diag, SIGNAL(projectionSettingsChanged()),
                    this, SLOT(onProjectionSettingsChanged_()));
            diag->show();
        });

    // ######### TOOLS MENU #########
    m = new QMenu(tr("Tools"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("Geometry editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            GeometryDialog * diag = new GeometryDialog(0, window_);
            connect(diag, SIGNAL(finished(int)), diag, SLOT(deleteLater()));
            diag->show();
        });

        a = new QAction(tr("Equation editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new EquationDisplayDialog(window_);
            connect(diag, SIGNAL(finished(int)), diag, SLOT(deleteLater()));
            diag->show();
        });


        a = new QAction(tr("Batch scene converter"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            SceneConvertDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Audio-filter tester"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            AudioFilterDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Timeline editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            TimelineEditDialog diag;
            diag.exec();
        });

    // ######### DEBUG MENU #########
    m = new QMenu(tr("Debug"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("QObject inspector"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            if (!qobjectInspector_)
                qobjectInspector_ = new QObjectInspector(application, window_);
            qobjectInspector_->setRootObject(this);
            qobjectInspector_->show();
        });

        m->addAction(a = new QAction(tr("Dump id names"), m));
        connect(a, SIGNAL(triggered()), SLOT(dumpIdNames_()));

        m->addAction( a = new QAction(tr("Dump object tree"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto tree = get_object_tree(scene_);
            tree->dumpTree(std::cout, "");
            delete tree;
        });

        m->addAction(a = new QAction(tr("Dump needed files"), m));
        connect(a, SIGNAL(triggered()), SLOT(dumpNeededFiles_()));

        m->addAction(a = new QAction(tr("Test transformation speed (new)"), m));
        connect(a, &QAction::triggered, [this](){ testSceneTransform_(true); });

        m->addAction(a = new QAction(tr("Test transformation speed (old)"), m));
        connect(a, &QAction::triggered, [this](){ testSceneTransform_(false); });

        m->addAction(a = new QAction(tr("Reset ObjectTreeModel"), m));
        connect(a, SIGNAL(triggered()), SLOT(resetTreeModel_()));

        m->addAction(a = new QAction(tr("Run test thread"), m));
        a->setCheckable(true);
        connect(a, SIGNAL(triggered()), SLOT(runTestThread_()));

        m->addAction(a = new QAction(tr("Export scene to povray"), m));
        connect(a, SIGNAL(triggered()), SLOT(exportPovray_()));

        m->addAction( a = new QAction(tr("Audio-link window"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new AudioLinkWindow(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->setScene(scene_);
            win->show();
        });

        m->addAction( a = new QAction(tr("Info window"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto w = new InfoWindow(window_);
            w->showFullScreen();
        });

        m->addAction( a = new QAction(tr("Dump modulation graph"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto tree = get_object_tree(scene_);
            tree->dumpTree(std::cout);

            ObjectGraph graph;
            getObjectModulatorGraph(graph, scene_);
            graph.dumpEdges(std::cout);
            QVector<Object*> linear;
            graph.makeLinear(std::inserter(linear, linear.begin()));
            std::cout << "linear: ";
            for (auto o : linear)
                std::cout << " " << o->idName();
            std::cout << std::endl;
        });

#ifndef MO_DISABLE_TREE
        m->addAction( a = new QAction(tr("Show modulation graph"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            ObjectGraph graph;
            getObjectModulatorGraph(graph, scene_);
            auto tree = new ObjectTreeNode(scene_);
                        //getObjectTree(scene_);
            //getModulationTree(tree, graph);

            //linearizedGraphToTree(tree, graph);
            TreeModel<Object*> model(tree);
            QDialog diag;
            auto l = new QVBoxLayout(&diag);
            auto tv = new QTreeView(&diag);
            l->addWidget(tv);
            tv->setModel(&model);
            diag.exec();
            delete tree;
        });
#endif

    // ######### HELP MENU #########
    m = new QMenu(tr("Help"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("Help"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            HelpDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Export help (html)"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), this, SLOT(exportHelpHtml_()));

        a = new QAction(tr("About Qt"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), application, SLOT(aboutQt()));


    // create projector-index-menu
    onProjectionSettingsChanged_();
}



void MainWidgetController::setScene_(Scene * s, const SceneSettings * set)
{
    MO_DEBUG_GUI("MainWidgetController::setScene_(" << s << ")");

    MO_ASSERT(s, "MainWidgetController::setScene_() with NULL scene");
    MO_ASSERT(s != scene_, "MainWidgetController::setScene_() with same scene");

    if (scene_)
    {
        scene_->kill();
        scene_->deleteLater();
    }

    scene_ = s;

    // manage memory
    scene_->setParent(this);

    // clear or init scene gui settings
    if (!set)
        sceneSettings_->clear();
    else
    {
        *sceneSettings_ = *set;
    }

#ifndef MO_DISABLE_TREE
    objectTreeModel_->setSceneSettings(sceneSettings_);
#endif
    objectGraphView_->setGuiSettings(sceneSettings_);

    // check for local filenames

    IO::FileList files;
    scene_->getNeededFiles(files);

    IO::fileManager().clear();
    IO::fileManager().addFilenames(files);
    IO::fileManager().acquireFiles();
    IO::fileManager().dumpStatus();

    MO_ASSERT(glManager_ && glWindow_, "");

    // set current resolution
    if (scene_->doMatchOutputResolution())
        scene_->setResolution(outputSize_);

    // set projection settings
    updateSceneProjectionSettings_();

    // update render settings from MainWidgetController
    updateDebugRender_();

    // connect to render window
    glManager_->setScene(scene_);
    connect(glManager_, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                scene_, SLOT(setFreeCameraMatrix(MO::Mat4)));

    // widgets -> scenetime
    connect(seqView_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));
    connect(sequencer_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));

    // scene changes
    connect(scene_, SIGNAL(numberOutputEnvelopesChanged(uint)),
            this, SLOT(updateNumberOutputEnvelopes_(uint)));
    connect(scene_, SIGNAL(outputEnvelopeChanged(const F32*)),
                    this, SLOT(updateOutputEnvelope_(const F32*)));
    connect(scene_, SIGNAL(sceneTimeChanged(Double)),
            this, SLOT(onSceneTimeChanged_(Double)));

    // update widgets

#ifndef MO_DISABLE_TREE
    scene_->setObjectModel(objectTreeModel_);
#endif
    scene_->setObjectEditor(objectEditor_);

#ifndef MO_DISABLE_TREE
    connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)),
            objectTreeView_, SLOT(columnMoved()/* force update */));
#endif
    connect(scene_, SIGNAL(parameterVisibilityChanged(MO::Parameter*)),
            objectView_, SLOT(updateParameterVisibility(MO::Parameter*)));

    objectGraphView_->setRootObject(scene_);
    objectView_->setObject(0);

    seqView_->setScene(scene_);
    seqView_->setSequence(0);

    sequencer_->setCurrentObject(scene_);
    clipView_->setClipContainer(0);

    glWindow_->renderLater();

    updateSystemInfo_();
    updateResolutionActions_();

    if (serverEngine().isRunning())
        serverEngine().sendScene(scene_);
}










void MainWidgetController::resetTreeModel_()
{
#ifndef MO_DISABLE_TREE
    objectTreeModel_->setSceneObject(scene_);
#endif
}

void MainWidgetController::onWindowKeyPressed_(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F7 && !isPlayback())
    {
        e->accept();
        start();
    }

    if (e->key() == Qt::Key_F8)
    {
        e->accept();
        stop();
    }

    if ((e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
        && e->modifiers() == 0)
    {
        e->accept();
        scene_->setFreeCameraIndex(e->key() - Qt::Key_0 - 1);
    }

    //if (!e->isAccepted())
    //    window_->keyPressEvent(e);
}




void MainWidgetController::onObjectAdded_(Object * o)
{
    // copy scene-gui-settings from original object (if pasted)
    copySceneSettings_(o);

    // update clipview
    if (Clip * clip = qobject_cast<Clip*>(o))
    {
        if (clip->clipContainer() == clipView_->clipContainer())
            clipView_->updateClip(clip);
    }

    // XXX refine this!
    onTreeChanged_();
}

void MainWidgetController::onObjectDeleted_(const Object * o)
{
    // update clipview
    clipView_->removeObject(o);

    // XXX refine this!
    onTreeChanged_();
}







void MainWidgetController::showClipView_(bool enable, Object * o)
{
    sequencer_->setVisible(!enable);
    clipView_->setVisible(enable);

    if (!enable)
        return;

    if (o->isClipContainer())
    {
        clipView_->setClipContainer(static_cast<ClipContainer*>(o));
    }
    else
    {
        Object * con = o->findParentObject(Object::T_CLIP_CONTAINER);
        if (con)
            clipView_->setClipContainer(static_cast<ClipContainer*>(con));

        clipView_->selectObject(o);
    }
}

void MainWidgetController::showSequencer_(bool enable, Object * o)
{
    sequencer_->setVisible(enable);
    clipView_->setVisible(!enable);

    if (!enable)
        return;

    sequencer_->setCurrentObject(o);
}

void MainWidgetController::showSequence_(bool enable, Sequence * seq)
{
    if (enable)
    {
        if (seq->type() == Object::T_SEQUENCE_FLOAT)
        {
            seqView_->setSequence(static_cast<SequenceFloat*>(seq));
            seqView_->setVisible(true);
        }
        else seqView_->setVisible(false);
    }
    else
    {
        seqView_->setVisible(false);
    }

}

/** Shows or hides the sequence view and selects a sequence to display */
void MainWidgetController::updateSequenceView_(Object * o)
{
    // show the clicked-on sequence
    if (o->isSequence())
        showSequence_(true, static_cast<Sequence*>(o));

    // selected a clip?
    if (o->isClip())
    {
        // if the clip contains the current displayed sequence
        if (seqView_->isVisible()
            && o->containsObject(seqView_->sequence()))
            return;

        // switch to first sequence
        auto clip = static_cast<Clip*>(o);
        if (!clip->sequences().isEmpty())
        {
            showSequence_(true, clip->sequences()[0]);
            return;
        }
    }
}


void MainWidgetController::onObjectNameChanged_(Object * o)
{
    // update clipview
    if (o->isClip())
        clipView_->updateClip(static_cast<Clip*>(o));

    // update objectview
    if (objectView_->object() == o)
        objectView_->updateObjectName();

    // flag as change to scene
    onSceneChanged_();

    // we don't need to update treeview here
}


void MainWidgetController::onObjectSelectedTree_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedTree(" << o << ")");

    // update object editor
    objectView_->setObject(o);

    if (!o)
    {
        showSequencer_(false);
        showSequence_(false);
        return;
    }

    // update sequence view
    updateSequenceView_(o);

    // update clipview
    if (o->isClip() || o->isClipContainer() ||
              o->findParentObject(Object::T_CLIP_CONTAINER))
    {
        showClipView_(true, o);
    }

    else
    {
        // update sequencer
        // when this object or it's real-object parent contains tracks
        Object * real = o->findContainingObject(Object::TG_REAL_OBJECT);
        if ((real && real->containsTypes(Object::TG_TRACK))
                || o->containsTypes(Object::TG_TRACK))
            showSequencer_(true, o);

        // if not tracks in object, select clips belonging to object
        if (clipView_->isVisible())
            clipView_->selectObject(o);
    }
}

void MainWidgetController::onObjectSelectedGraphView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedGraphView(" << o << ")");

    // update object editor
    objectView_->setObject(o);

    if (!o)
    {
        showSequencer_(false);
        showSequence_(false);
        return;
    }
#ifndef MO_DISABLE_TREE
    else
        objectTreeView_->setFocusIndex(o);
#endif

    // update sequence view
    updateSequenceView_(o);

    // update clipview
    if (o->isClip() || o->isClipContainer() ||
              o->findParentObject(Object::T_CLIP_CONTAINER))
    {
        showClipView_(true, o);
    }

    else
    {
        // update sequencer
        // when this object or it's real-object parent contains tracks
        Object * real = o->findContainingObject(Object::TG_REAL_OBJECT);
        if ((real && real->containsTypes(Object::TG_TRACK))
                || o->containsTypes(Object::TG_TRACK))
            showSequencer_(true, o);

        // if not tracks in object, select clips belonging to object
        if (clipView_->isVisible())
            clipView_->selectObject(o);
    }
}

void MainWidgetController::onObjectSelectedClipView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::onObjectSelectedClipView_(" << o << ")");

    // update object editor
    objectView_->setObject(o);

    // jump to clip in graph view
    objectGraphView()->setFocusObject(o);
    objectView()->selectObject(o);

    // update sequence view
    updateSequenceView_(o);
}


void MainWidgetController::onObjectSelectedObjectView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedObjectView_(" << o << ")");

    // update sequence view
    updateSequenceView_(o);

    // jump to modulator in tree view
    objectGraphView()->setFocusObject(o);

    // update clipview
    if (o && (o->isClip() || o->isClipContainer() ||
              o->findParentObject(Object::T_CLIP_CONTAINER)))
    {
        showClipView_(true, o);
    }

    // update sequencer
    else
    {
        showSequencer_(true, o);
    }
}


void MainWidgetController::onObjectSelectedSequencer_(Sequence * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedSequencer_(" << o << ")");

    // update object editor
    objectView_->setObject(o);

    // jump to sequence in tree view
    objectGraphView()->setFocusObject(o);
    objectView()->selectObject(o);
}

void MainWidgetController::onSequenceClicked_()
{
    // update objectview if not already
    if (objectView_->object() != seqView_->sequence())
        objectView_->setObject(seqView_->sequence());

    objectGraphView()->setFocusObject(seqView_->sequence());
    objectView()->selectObject(seqView_->sequence());
}


void MainWidgetController::onTreeChanged_()
{
    //objectTreeView_->updateFromModel();

    sequencer_->setCurrentObject(scene_);
}

void MainWidgetController::onSceneChanged_()
{
    if (!sceneNotSaved_)
    {
        sceneNotSaved_ = true;
        updateWindowTitle_();
    }
}

void MainWidgetController::onSceneTimeChanged_(Double time)
{
    if (seqView_->isVisible())
        seqView_->setSceneTime(time);
    if (sequencer_->isVisible())
        sequencer_->setSceneTime(time);

    transportWidget_->setSceneTime(time);
}

void MainWidgetController::setEditActions_(const QObject *, QList<QAction *> actions)
{
#ifndef __APPLE__
    menuEdit_->clear();
    menuEdit_->addActions(actions);
#endif
}

void MainWidgetController::testSceneTransform_(bool newVersion)
{
    QTime t;

    int num = 10000000;
    int i = 0;
    int e = 0;

    if (!newVersion)
    {
        t.start();
        for (; i < num && e <= 1000;)
        {
            for (int j=0; j<1000; ++j, ++i)
                scene_->calculateSceneTransform(0, 0, scene_->sampleRateInv() * (i*1000+j));

            e = t.elapsed();
        }
        num = i;
        const Double elapsed = (Double)e / 1000.0;

        QMessageBox::information(window_, tr("Scene transformation test"),
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
    else
    {
        const auto bufsize = scene_->bufferSize(MO_AUDIO_THREAD);

        ObjectDspPath dsp;
        dsp.createPath(scene_, scene_->sampleRate(), bufsize);

        t.start();
        for (; i < num && e <= 1000; )
        {
            for (int j=0; j<20; ++j, i += bufsize)
                dsp.calcTransformations(i*1000+j, MO_AUDIO_THREAD);

            e = t.elapsed();
        }
        num = i;
        const Double elapsed = (Double)e / 1000.0;

        QMessageBox::information(window_, tr("Scene transformation test"),
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
}

void MainWidgetController::runTestThread_()
{
    if (!testThread_)
    {
        testThread_ = new TestThread(scene_, true, this);
        testThread_->start();
    }
    else
    {
        testThread_->stop();
        testThread_->deleteLater();
        testThread_ = 0;
    }
}

void MainWidgetController::dumpIdNames_()
{
    QSet<QString> ids = scene_->getChildIds(true);
    ids.insert(scene_->idName());

    std::set<QString> sorted;

    for (auto & id : ids)
        sorted.insert(id);

    for (auto & id : sorted)
        MO_PRINT("{" << id << "}");
}

void MainWidgetController::dumpNeededFiles_()
{
    IO::FileList files;
    scene_->getNeededFiles(files);

    for (const IO::FileListEntry& f : files)
    {
        MO_PRINT(IO::fileTypeNames[f.second] << "\t " << f.first);
    }
}

void MainWidgetController::updateSystemInfo_()
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

void MainWidgetController::updateWindowTitle_()
{
    QString t;

    if (sceneNotSaved_)
        t = "* ";

    if (!currentSceneFilename_.isEmpty())
    {
        t += QFileInfo(currentSceneFilename_).fileName() + " - ";
    }

    emit windowTitle(t + applicationName());
}

void MainWidgetController::updateWidgetsActivity_()
{
    actionSaveScene_->setEnabled( !currentSceneFilename_.isEmpty() );
}

void MainWidgetController::copySceneSettings_(Object *o)
{
    if (o->idName() != o->originalIdName())
        sceneSettings_->copySettings(o->idName(), o->originalIdName());

    for (auto c : o->childObjects())
        copySceneSettings_(c);
}


void MainWidgetController::updateDebugRender_()
{
    if (!scene_)
        return;

    scene_->setDebugRenderOptions(
          (Scene::DD_AUDIO_SOURCES * aDrawAudioSources_->isChecked())
        | (Scene::DD_CAMERAS * aDrawCameras_->isChecked())
        | (Scene::DD_LIGHT_SOURCES * aDrawLightSources_->isChecked())
        | (Scene::DD_MICROPHONES * aDrawMicrophones_->isChecked())
                );
}

bool MainWidgetController::isPlayback() const
{
    return scene_ && scene_->isPlayback();
}

void MainWidgetController::start()
{
    scene_->start();

    if (serverEngine().isRunning())
        serverEngine().setScenePlaying(true);
}

void MainWidgetController::stop()
{
    scene_->stop();

    if (serverEngine().isRunning())
        serverEngine().setScenePlaying(false);
}

void MainWidgetController::updateNumberOutputEnvelopes_(uint num)
{
    transportWidget_->envelopeWidget()->setNumberChannels(num);
}

void MainWidgetController::updateOutputEnvelope_(const F32 * l)
{
    transportWidget_->envelopeWidget()->setLevel(l);
    transportWidget_->envelopeWidget()->update();
}



void MainWidgetController::newScene()
{
    if (isPlayback())
        stop();

    if (!isOkayToChangeScene())
        return;

    setScene_( ObjectFactory::createSceneObject() );
    currentSceneFilename_.clear();
    IO::Files::setFilename(IO::FT_SCENE, "");
    sceneNotSaved_ = false;
    updateWindowTitle_();
    updateWidgetsActivity_();
}

bool MainWidgetController::isOkayToChangeScene()
{
    if (isPlayback())
        stop();

    if (!sceneNotSaved_)
        return true;

    QMessageBox::StandardButton res =
    QMessageBox::question(window_, tr("Project not saved"),
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



void MainWidgetController::initScene()
{
    try
    {
        QString fn = IO::Files::filename(IO::FT_SCENE);
        if (!fn.isEmpty())
            loadScene_(fn);
        else
            newScene();
    }
    catch (IoException& e)
    {
        MO_WARNING(e.what());
        newScene();
    }
}

QString MainWidgetController::getSceneSaveFilename_()
{
    if (isPlayback())
        stop();

    return IO::Files::getSaveFileName(IO::FT_SCENE, window_);
}

bool MainWidgetController::saveScene()
{
    if (!scene_)
        return false;

    QString fn = currentSceneFilename_;

    if (fn.isEmpty())
        fn = getSceneSaveFilename_();

    return saveScene_(fn);
}

void MainWidgetController::saveSceneAs()
{
    if (!scene_)
        return;

    QString fn = getSceneSaveFilename_();

    saveScene_(fn);
}


void MainWidgetController::loadScene()
{
    if (isPlayback())
        stop();

    if (!isOkayToChangeScene())
        return;

    QString fn = IO::Files::getOpenFileName(IO::FT_SCENE, window_);

    loadScene_(fn);
}

void MainWidgetController::loadScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        // read scene file
        Scene * scene;
        try
        {
            scene = ObjectFactory::loadScene(fn);
        }
        catch (Exception& e)
        {
            QMessageBox::critical(window_, tr("load scene failed"),
                                  tr("Could not open project '%1'\n%2").arg(fn).arg(e.what()));

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
                sceneSettings.updateTreeForCompatibility(scene);
            }
            else
                MO_DEBUG_IO("No scene-settings " << guifn);
        }
        catch (Exception & e) { }

        setScene_(scene, &sceneSettings);

        statusBar_->showMessage(tr("Opened %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        sceneNotSaved_ = false;
        updateWindowTitle_();
        updateWidgetsActivity_();
    }
    else
        statusBar()->showMessage(tr("loading cancelled"), statusMessageTimeout_);
}

bool MainWidgetController::saveScene_(const QString &fn)
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
        catch (const Exception & e)
        {
            statusBar()->showMessage(tr("SAVING FAILED! %1").arg(e.what()));
            return false;
        }

        statusBar()->showMessage(tr("Saved %1").arg(fn), statusMessageTimeout_);
        currentSceneFilename_ = fn;
        sceneNotSaved_ = false;
        updateWindowTitle_();
        updateWidgetsActivity_();
        return true;
    }

    statusBar()->showMessage(tr("saving cancelled"), statusMessageTimeout_);
    return false;
}


void MainWidgetController::renderToDisk()
{
    auto ren = new Renderer(this);

    ren->setScene(scene_);
    ren->setOutputPath("/home/defgsus/prog/qt_project/mo/matrixoptimizer/render");

    if (!ren->prepareRendering())
    {
        QMessageBox::critical(window_, tr("Render to disk"),
                              tr("Sorry, but rendering to disk failed"));
        ren->deleteLater();
        return;
    }

    connect(ren, SIGNAL(finished()), ren, SLOT(deleteLater()));

    MO_DEBUG_RENDER("starting renderer");
    ren->start();
}

void MainWidgetController::exportPovray_()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_POVRAY, window_);
    if (fn.isEmpty())
        return;

    IO::PovrayExporter pov;
    pov.setScene(scene_);
    pov.exportScene(fn, scene_->sceneTime());
}

void MainWidgetController::onResolutionOutput_()
{
    if (scene_)
    {
        // notify change on scene
        if (!scene_->doMatchOutputResolution())
            onSceneChanged_();

        scene_->setMatchOutputResolution(true);
        if (scene_->frameBufferSize() != outputSize_)
            scene_->setResolution(outputSize_);
    }
}

void MainWidgetController::onResolutionCustom_()
{
    ResolutionDialog diag(scene_->frameBufferSize());

    if (diag.exec() != QDialog::Accepted)
        return;

    if (scene_)
    {
        // notify change on scene
        if (scene_->doMatchOutputResolution()
            || scene_->frameBufferSize() != diag.resolution())
            onSceneChanged_();

        scene_->setMatchOutputResolution(false);
        scene_->setResolution(diag.resolution());
    }

    updateResolutionActions_();
}

void MainWidgetController::onResolutionPredefined_(QAction * a)
{
    setPredefinedResolution_(a->data().toInt());
}

void MainWidgetController::setPredefinedResolution_(int index)
{
    if (index < 0 || index >= CommonResolutions::resolutions.size())
        return;

    QSize res = CommonResolutions::resolutions[index].size();

    aResolutionPredefined_->setChecked(true);
    aResolutionCustom_->setText(tr("Custom ..."));

    if (scene_)
    {
        // notify change on scene
        if (!scene_->doMatchOutputResolution()
            || scene_->frameBufferSize() != res)
            onSceneChanged_();

        scene_->setMatchOutputResolution(false);
        scene_->setResolution(res);
    }
}

void MainWidgetController::updateResolutionActions_()
{
    if (!scene_)
        return;

    // update from output window resolution
    aResolutionOutput_->setText(tr("Same as output window %1x%2")
                                .arg(outputSize_.width())
                                .arg(outputSize_.height()));

    if (scene_->doMatchOutputResolution())
    {
        aResolutionOutput_->setChecked(true);
        aResolutionCustom_->setText(tr("Custom ..."));
    }
    else
    {
        bool found = false;

        // find current scene resolution in predefined menu
        for (QAction * a : menuResolutions_->actions())
        {
            const int index = a->data().toInt();
            if (index >= 0 && index < CommonResolutions::resolutions.size())
            {
                const QSize s = CommonResolutions::resolutions[index].size();
                if (s == scene_->requestedFrameBufferSize())
                {
                    a->setChecked(found = true);
                    aResolutionPredefined_->setChecked(true);
                    break;
                }
            }
        }

        // set custom resolution
        if (!found)
        {
            aResolutionCustom_->setChecked(true);
            aResolutionCustom_->setText(tr("Custom %1x%2 ...")
                                        .arg(scene_->requestedFrameBufferSize().width())
                                        .arg(scene_->requestedFrameBufferSize().height()));
        }
    }
}

void MainWidgetController::onProjectionSettingsChanged_()
{
    // update scene
    updateSceneProjectionSettings_();

    // update list of projectors in menu

    menuProjectorIndex_->clear();

    auto g = new QActionGroup(menuProjectorIndex_);

    const ProjectionSystemSettings& set = settings->getDefaultProjectionSettings();
    for (uint i=0; i<set.numProjectors(); ++i)
    {
        auto a = new QAction(menuProjectorIndex_);
        a->setText(QString("%1 - %2")
                   .arg(i+1)
                   .arg(set.projectorSettings(i).name()));
        a->setData(i);
        a->setCheckable(true);
        a->setChecked(settings->clientIndex() == (int)i);

        menuProjectorIndex_->addAction(a);
        g->addAction(a);
    }

}

void MainWidgetController::updateSceneProjectionSettings_()
{
    if (!scene_)
        return;

    scene_->setProjectionSettings(settings->getDefaultProjectionSettings());
    scene_->setProjectorIndex(settings->clientIndex());
}

void MainWidgetController::onOutputSizeChanged_(const QSize & size)
{
    outputSize_ = size;

    if (aResolutionOutput_ != 0)
    {
        aResolutionOutput_->setText(tr("Same as output window %1x%2")
                                .arg(outputSize_.width())
                                .arg(outputSize_.height()));
    }

    // link scene resolution to window resolution
    if (scene_ && scene_->doMatchOutputResolution())
        scene_->setResolution(outputSize_);
}

void MainWidgetController::exportHelpHtml_()
{
    QString fn = IO::Files::getOpenDirectory(IO::FT_HELP_EXPORT);
    if (fn.isEmpty())
        return;

    HelpExporterHtml exp;
    try
    {
        exp.save(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(0, tr("Help export"),
                              tr("Failed to export html help to '%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
}



void MainWidgetController::sendSceneToClients_()
{
    if (!scene_)
        return;

    serverEngine().sendScene(scene_);
}



} // namespace GUI
} // namespace MO
