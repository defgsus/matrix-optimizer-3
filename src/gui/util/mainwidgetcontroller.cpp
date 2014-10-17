/** @file mainwidgetcontroller.cpp

    @brief Controller for all main edit widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 17.10.2014</p>
*/

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

#include "mainwidgetcontroller.h"
#include "io/error.h"
#include "io/log.h"
#include "io/settings.h"
#include "io/application.h"
#include "io/memory.h"
#include "io/version.h"
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




namespace MO {
namespace GUI {

namespace {
    static QString windowTitleString("Matrix Optimizer " + versionString());
}



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
        setCurrentThreadName("AUDIOTEST");

        samplePos_ = 0;
        while (!stop_)
        {
            scene_->calculateAudioBlock(samplePos_, MO_AUDIO_THREAD);
            samplePos_ += scene_->bufferSize(MO_AUDIO_THREAD);

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



























MainWidgetController::MainWidgetController(QMainWindow * win)
    : QObject           (win),
      window_           (win),
      scene_            (0),
      objectTreeModel_  (0),
      glManager_        (0),
      glWindow_         (0),
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
    setObjectName("_MainWidgetController");

    createObjects_();
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

    // status bar
    statusBar_ = new QStatusBar(window_);
    statusBar_->addPermanentWidget(sysInfoLabel_ = new QLabel(statusBar()));

    // transport widget
    transportWidget_ = new TransportWidget(window_);

    // object tree view
    objectTreeView_ = new ObjectTreeView(window_);
    objectTreeView_->setSceneSettings(sceneSettings_);
    connect(objectTreeView_, SIGNAL(editActionsChanged(const QObject*,QList<QAction*>)),
            SLOT(setEditActions_(const QObject*,QList<QAction*>)));
    connect(objectTreeView_, SIGNAL(objectSelected(MO::Object*)),
            SLOT(onObjectSelectedTree_(MO::Object*)));

    // object tree model
    objectTreeModel_ = new ObjectTreeModel(0, this);
    //connect(objectTreeModel_, SIGNAL(sceneChanged()),
    //        this, SLOT(onSceneChanged_()));
    objectTreeView_->setObjectModel(objectTreeModel_);

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
    sequencer_->setMinimumHeight(300);
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
    m = editMenu_ = new QMenu(tr("Edit"), menuBar);
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

        // ##### DEBUG VISIBILITY SUBMENU #####
        auto sub = new QMenu(tr("Visibility"), menuBar);
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
                serverDialog_ = new ServerDialog(window_);
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
            ProjectorSetupDialog diag(window_);
            diag.exec();
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

        m->addAction(a = new QAction(tr("Dump needed files"), m));
        connect(a, SIGNAL(triggered()), SLOT(dumpNeededFiles_()));

        m->addAction(a = new QAction(tr("Test transformation speed"), m));
        connect(a, SIGNAL(triggered()), SLOT(testSceneTransform_()));

        m->addAction(a = new QAction(tr("Reset ObjectTreeModel"), m));
        connect(a, SIGNAL(triggered()), SLOT(resetTreeModel_()));

        m->addAction(a = new QAction(tr("Run test thread"), m));
        a->setCheckable(true);
        connect(a, SIGNAL(triggered()), SLOT(runTestThread_()));

        m->addAction(a = new QAction(tr("Export scene to povray"), m));
        connect(a, SIGNAL(triggered()), SLOT(exportPovray_()));

        a = new QAction(tr("Audio-link window"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new AudioLinkWindow(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->setScene(scene_);
            win->show();
        });

        a = new QAction(tr("Info window"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto w = new InfoWindow(window_);
            w->showFullScreen();
        });


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

        a = new QAction(tr("About Qt"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), application, SLOT(aboutQt()));
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

    // clear or init scene widget settings
    if (!set)
        sceneSettings_->clear();
    else
        *sceneSettings_ = *set;

    MO_ASSERT(glManager_ && glWindow_, "");

    // update render settings from MainWidgetController
    updateDebugRender_();

    // connect to render window
    connect(glManager_, SIGNAL(renderRequest(uint)), scene_, SLOT(renderScene(uint)));
    connect(glManager_, SIGNAL(contextCreated(uint,MO::GL::Context*)),
                scene_, SLOT(setGlContext(uint,MO::GL::Context*)));
    connect(glManager_, SIGNAL(cameraMatrixChanged(MO::Mat4)),
                scene_, SLOT(setFreeCameraMatrix(MO::Mat4)));

    connect(scene_, SIGNAL(renderRequest()), glWindow_, SLOT(renderLater()));

    if (glWindow_->context())
        scene_->setGlContext(glWindow_->threadId(), glWindow_->context());
    connect(scene_, SIGNAL(playbackStarted()),
            glWindow_, SLOT(startAnimation()));
    connect(scene_, SIGNAL(playbackStopped()),
            glWindow_, SLOT(stopAnimation()));

    connect(seqView_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));
    connect(sequencer_, SIGNAL(sceneTimeChanged(Double)),
            scene_, SLOT(setSceneTime(Double)));

    // scene changes
    // XXX still very unfinished right now - but in the process right now ;)
    connect(scene_, SIGNAL(objectNameChanged(MO::Object*)), this, SLOT(onObjectNameChanged_(MO::Object*)));
    connect(scene_, SIGNAL(objectAdded(MO::Object*)), this, SLOT(onObjectAdded_(MO::Object*)));
    connect(scene_, SIGNAL(objectDeleted(const MO::Object*)), this, SLOT(onObjectDeleted_(const MO::Object*)));
    connect(scene_, SIGNAL(childrenSwapped(MO::Object*,int,int)), this, SLOT(onTreeChanged_()));
    connect(scene_, SIGNAL(sequenceChanged(MO::Sequence*)), this, SLOT(onSceneChanged_()));
    connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)), this, SLOT(onSceneChanged_()));
    connect(scene_, SIGNAL(numberOutputEnvelopesChanged(uint)),
            this, SLOT(updateNumberOutputEnvelopes_(uint)));
    connect(scene_, SIGNAL(outputEnvelopeChanged(const F32*)),
                    this, SLOT(updateOutputEnvelope_(const F32*)));

    // update widgets

    scene_->setObjectModel(objectTreeModel_);
    connect(scene_, SIGNAL(sceneTimeChanged(Double)),
            seqView_, SLOT(setSceneTime(Double)));
    connect(scene_, SIGNAL(sceneTimeChanged(Double)),
            sequencer_, SLOT(setSceneTime(Double)));
    connect(scene_, SIGNAL(parameterChanged(MO::Parameter*)),
            objectTreeView_, SLOT(columnMoved()/* force update */));
    connect(scene_, SIGNAL(parameterVisibilityChanged(MO::Parameter*)),
            objectView_, SLOT(updateParameterVisibility(MO::Parameter*)));

    objectView_->setObject(0);

    seqView_->setScene(scene_);
    seqView_->setSequence(0);

    sequencer_->setCurrentObject(scene_);
    clipView_->setClipContainer(0);

    glWindow_->renderLater();

    updateSystemInfo_();

    if (serverEngine().isRunning())
        serverEngine().sendScene(scene_);
}










void MainWidgetController::resetTreeModel_()
{
    objectTreeModel_->setSceneObject(scene_);
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
    objectView_->setObject(o);

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


void MainWidgetController::onObjectSelectedClipView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::onObjectSelectedClipView_(" << o << ")");

    // update object editor
    objectView_->setObject(o);

    // jump to clip in tree view
    objectTreeView_->setFocusIndex(o);

    // update sequence view
    updateSequenceView_(o);
}

void MainWidgetController::onObjectSelectedObjectView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedObjectView_(" << o << ")");

    // update sequence view
    updateSequenceView_(o);

    // jump to modulator in tree view
    objectTreeView_->setFocusIndex(o);

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
    objectTreeView_->setFocusIndex(o);
}

void MainWidgetController::onSequenceClicked_()
{
    // update objectview if not already
    if (objectView_->object() != seqView_->sequence())
        objectView_->setObject(seqView_->sequence());

    // focus in tree
    const QModelIndex & idx = objectTreeView_->getIndexForObject(seqView_->sequence());
    if (objectTreeView_->currentIndex() != idx)
        objectTreeView_->setFocusIndex(idx);
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

void MainWidgetController::setEditActions_(const QObject *, QList<QAction *> actions)
{
#ifndef __APPLE__
    editMenu_->clear();
    editMenu_->addActions(actions);
#endif
}

void MainWidgetController::testSceneTransform_()
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

void MainWidgetController::runTestThread_()
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

    emit windowTitle(t + windowTitleString);
}

void MainWidgetController::updateWidgetsActivity_()
{
    actionSaveScene_->setEnabled( !currentSceneFilename_.isEmpty() );
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
}

void MainWidgetController::stop()
{
    scene_->stop();
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








} // namespace GUI
} // namespace MO
