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
#include <QPushButton>

#include "mainwidgetcontroller.h"
#include "io/error.h"
#include "io/log_gui.h"
#include "io/log_io.h"
#include "io/settings.h"
#include "io/application.h"
#include "io/memory.h"
#include "io/version.h"
#include "io/filemanager.h"
#include "io/helpsystem.h" // for setHelpUrl()
#include "io/helpexporterhtml.h"
#include "io/currenttime.h"
#include "gui/timeline1dview.h"
#include "gui/timeline1drulerview.h"
#include "gui/ruler.h"
#include "gui/qobjectinspector.h"
#include "gui/objectview.h"
#include "gui/sequenceview.h"
#include "gui/sequencer.h"
#include "gui/clipview.h"
#include "gui/helpdialog.h"
#include "gui/audiodialog.h"
#include "gui/geometrydialog.h"
#include "gui/equationdisplaydialog.h"
#include "gui/sceneconvertdialog.h"
#include "gui/projectorsetupdialog.h"
#include "gui/networkdialog.h"
#include "gui/midisettingsdialog.h"
#include "gui/infowindow.h"
#include "gui/timelineeditdialog.h"
#include "gui/audiofilterdialog.h"
#include "gui/scenedescdialog.h"
#include "gui/distancefieldimage.h"
#include "gui/polygonimageimporter.h"
#include "gui/widget/assetbrowser.h"
#include "gui/widget/objectoutputview.h"
#include "gui/imagelistdialog.h"
#include "gui/csgdialog.h"
#include "gui/evolutiondialog.h"
#ifndef MO_DISABLE_SERVER
#   include "gui/serverview.h"
#   include "engine/serverengine.h"
#endif
#include "gui/resolutiondialog.h"
#include "gui/objectgraphview.h"
#include "gui/frontview.h"
#include "gui/frontitemeditor.h"
#include "gui/util/frontscene.h"
#include "gui/util/frontpreset.h"
#include "gui/util/recentfiles.h"
#include "tool/actionlist.h"
#include "gui/widget/envelopewidget.h"
#include "gui/widget/transportwidget.h"
#include "gui/bulkrenamedialog.h"
#include "gui/oscview.h"
#include "gui/widget/objecttreeview.h"
#include "model/objecttreemodel.h"
#ifndef MO_DISABLE_ANGLESCRIPT
#   include "gui/widget/angelscriptwidget.h"
#   include "script/angelscript.h"
#   include "script/angelscript_object.h"
#endif
#ifdef MO_ENABLE_PYTHON34
#   include "gui/widget/python34widget.h"
#endif
#include "gui/util/scenesettings.h"
#include "gui/texteditdialog.h"
#include "gui/renderdialog.h"
#ifndef MO_HAMBURG
#   include "gui/wavetracerdialog.h"
#   include "io/sswproject.h"
#   include "gui/sswimporter.h"
#endif
#ifndef MO_DISABLE_LADSPA
#   include "gui/audioplugindialog.h"
#endif
#include "io/datastream.h"
#include "io/files.h"
#include "io/povrayexporter.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "gl/texture.h"
#include "gl/scenerenderer.h" // deprecated
#include "gl/compatibility.h"
#include "audio/configuration.h"
#include "engine/audioengine.h"
#include "engine/liveaudioengine.h"
#include "engine/serverengine.h"
#include "object/util/objectfactory.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/control/sequencefloat.h"
#include "object/control/clipcontroller.h"
#include "object/util/objectmodulatorgraph.h"
#include "object/util/objecteditor.h"
#include "object/util/objectdsppath.h"
#include "object/util/objecttree.h"
#include "object/util/scenesignals.h"
#include "object/interface/valuefloatinterface.h"
#include "tool/commonresolutions.h"

namespace MO {
namespace GUI {

const int MAX_RECENT_FILES = 42;


MainWidgetController::MainWidgetController(QMainWindow * win)
    : QObject           (win),
      window_           (win),
      scene_            (0),
      updateTimer_      (0),
      outputSize_       (512, 512),
      audioEngine_      (0),
      //renderEngine_     (0),
      glManager_        (0),
      glWindow_         (0),
      objectEditor_     (0),
      sceneSettings_    (0),
      objectTreeView_   (0),
      objectView_       (0),
      sequencer_        (0),
      clipView_         (0),
      seqView_          (0),
#ifndef MO_DISABLE_FRONT
      frontScene_       (0),
      frontView_        (0),
      frontItemEditor_  (0),
#endif
      objectOutputView_ (0),
      assetBrowser_     (0),
      transportWidget_  (0),
      isVisibleSequencer_(true),
      isVisibleClipView_(true),
      isVisibleSeqView_ (true),
      //qobjectInspector_ (0),
      serverView_       (0),
      statusBar_        (0),
      sysInfoLabel_     (0),
      recentFiles_      (0),
      aGlWindowVisible_ (0),
      sceneNotSaved_    (false),
      statusMessageTimeout_(7 * 1000),
      timeStep1_        (1.0),
      timeStep2_        (10.0)
{
    aResolutionOutput_ = 0;
    setObjectName("_MainWidgetController");

    createObjects_();

    // start server automatically
#ifndef MO_DISABLE_SERVER
    if (isServer())
        if (settings()->value("Server/running").toBool()
            && !serverEngine().isRunning())
                serverEngine().open();
#endif
}

MainWidgetController::~MainWidgetController()
{
    // store settings

    recentFiles_->saveSettings();

    // shut down

    //if (renderEngine_)
    //    renderEngine_->release();
    //delete renderEngine_;

    if (audioEngine_)
        audioEngine_->stop();
    delete audioEngine_;

    if (scene_)
    {
        glManager_->setScene(nullptr);
        //scene_->kill();
        scene_->releaseRef("gui shutdown");
    }

    delete glManager_;
}

void MainWidgetController::createObjects_()
{
    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(1000 / 20);
    updateTimer_->setSingleShot(false);
    connect(updateTimer_, SIGNAL(timeout()), this, SLOT(onUpdateTimer_()));

    // scene settings class
    sceneSettings_ = new SceneSettings(this);

    // editor
    objectEditor_ = new ObjectEditor(this);
    connect(objectEditor_, SIGNAL(objectNameChanged(MO::Object*)), this, SLOT(onObjectNameChanged_(MO::Object*)));
    connect(objectEditor_, SIGNAL(objectAdded(MO::Object*)), this, SLOT(onObjectAdded_(MO::Object*)));
    connect(objectEditor_, SIGNAL(objectDeleted(const MO::Object*)), this, SLOT(onObjectDeleted_(const MO::Object*)));
    connect(objectEditor_, SIGNAL(objectsDeleted(QList<MO::Object*>)), this, SLOT(onObjectsDeleted_(QList<MO::Object*>)));
    connect(objectEditor_, SIGNAL(sequenceChanged(MO::Sequence*)), this, SLOT(onSceneChanged_()));
    connect(objectEditor_, SIGNAL(valueFloatChanged(MO::ValueFloatInterface*)), this, SLOT(onValueFloatChanged_()));
    connect(objectEditor_, SIGNAL(parameterChanged(MO::Parameter*)), this, SLOT(onParamChanged_()));
    connect(objectEditor_, SIGNAL(parameterChanged(MO::Parameter*)), this, SLOT(onSceneChanged_()));
    connect(objectEditor_, SIGNAL(parametersChanged(MO::Object*)), this, SLOT(onParamsChanged_(MO::Object*)));
    connect(objectEditor_, SIGNAL(parameterVisibilityChanged(MO::Parameter*)), this, SLOT(onParamVisChanged_()));
    connect(objectEditor_, &ObjectEditor::sceneChanged, [=](MO::Scene * s)
    {
        if (audioEngine_)
            audioEngine_->setScene(s, MO_AUDIO_THREAD);
    });

    // status bar
    statusBar_ = new QStatusBar(window_);
    statusBar_->addPermanentWidget(sysInfoLabel_ = new QLabel(statusBar()));

    // transport widget
    transportWidget_ = new TransportWidget(window_);
    connect(transportWidget_, &TransportWidget::transportBack2, [=](){ moveTime(-timeStep2_); });
    connect(transportWidget_, &TransportWidget::transportBack1, [=](){ moveTime(-timeStep1_); });
    connect(transportWidget_, &TransportWidget::transportPlay, [=](){ start(); });
    connect(transportWidget_, &TransportWidget::transportStop, [=](){ stop(); });
    connect(transportWidget_, &TransportWidget::transportForward1, [=](){ moveTime(timeStep1_); });
    connect(transportWidget_, &TransportWidget::transportForward2, [=](){ moveTime(timeStep2_); });

    // object graph view
    objectGraphView_ = new ObjectGraphView(window_);
    connect(objectGraphView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedGraphView_(MO::Object*)));
    connect(objectGraphView_, &ObjectGraphView::editActionsChanged, [=](const ActionList& a)
    {
        setEditActions_(objectGraphView_, a);
    });

    // tree view
    objectTreeView_ = new ObjectTreeView(window_);
    connect(objectTreeView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedTree_(MO::Object*)));

    // object (parameter) View
    objectView_ = new ObjectView(window_);
    connect(objectView_, SIGNAL(objectSelected(MO::Object*)),
            this, SLOT(onObjectSelectedObjectView_(MO::Object*)));
    connect(objectView_, SIGNAL(statusTipChanged(QString)),
            statusBar_, SLOT(showMessage(QString)));

#ifndef MO_DISABLE_FRONT
    // front-end scene
    frontScene_ = new FrontScene(window_);
    frontScene_->setObjectEditor(objectEditor_);
    connect(frontScene_, SIGNAL(sceneChanged()), this, SLOT(onSceneChanged_()));
    connect(frontScene_, SIGNAL(editModeChanged(bool)), this, SLOT(onUiEditModeChanged_(bool)));
    connect(frontScene_, &FrontScene::actionsChanged, [=](const QList<QAction*>& a)
    {
        /** @todo fix setEditActions_ from FrontScene */
        //setEditActions_(frontScene_, a);
    });
    // front-end view
    frontView_ = new FrontView(window_);
    frontView_->setFrontScene(frontScene_);
    // front-end properties
    frontItemEditor_ = new FrontItemEditor(window_);
    connect(frontScene_, SIGNAL(itemSelected(AbstractFrontItem*)),
            frontItemEditor_, SLOT(setItem(AbstractFrontItem*)));
    connect(frontScene_, SIGNAL(itemsSelected(QList<AbstractFrontItem*>)),
            frontItemEditor_, SLOT(setItems(QList<AbstractFrontItem*>)));
    connect(frontScene_, &FrontScene::itemUnselected, [=]()
    {
        frontItemEditor_->setItem(0);
    });
    connect(frontScene_, &FrontScene::itemsDeleted, [=](const QList<QString>& ids)
    {
        frontItemEditor_->setItem(0);
        if (objectEditor_->scene())
            objectEditor_->removeUiModulators(ids);
    });
#endif

    // sequencer
    sequencer_ = new Sequencer(window_);
    sequencer_->setMinimumHeight(320);
    sequencer_->setVisible(false);
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

    // asset browser
    assetBrowser_ = new AssetBrowser(window_);
    connect(assetBrowser_, &AssetBrowser::sceneSelected, [=](QString fn){ loadScene(fn); });

    // server/client view
    serverView_ = 0;
#ifndef MO_DISABLE_SERVER
    if (isServer())
    {
        serverView_ = new ServerView(window_);
        connect(serverView_, SIGNAL(sendScene()),
                this, SLOT(sendSceneToClients_()));
    }
#endif

    oscView_ = new OscView(window_);

    // gl manager and window
    glManager_ = new GL::Manager(this);
    connect(glManager_, SIGNAL(outputSizeChanged(QSize)),
            this, SLOT(onOutputSizeChanged_(QSize)));
    glManager_->setTimeCallback(
                [this](){ return audioEngine_ ? audioEngine_->second() : 0.0; });

    /*
    glWindow_ = glManager_->createGlWindow(MO_GFX_THREAD);
    connect(glWindow_, SIGNAL(visibleChanged(bool)),
            this, SLOT(onGlWindowVisibleChanged_(bool)));
    connect(glWindow_, SIGNAL(keyPressed(QKeyEvent*)),
            this, SLOT(onWindowKeyPressed_(QKeyEvent*)));
    glWindow_->show();
    */

    // object output view
    objectOutputView_ = new ObjectOutputView(window_);

    // sysinfo at some interval
    sysInfoTimer_ = new QTimer(this);
    sysInfoTimer_->setInterval(5000);
    connect(sysInfoTimer_, SIGNAL(timeout()), this, SLOT(updateSystemInfo_()));
    sysInfoTimer_->start();

    updateSystemInfo_();
}


void MainWidgetController::createMainMenu(QMenuBar * menuBar)
{
    recentFiles_ = new RecentFiles(MAX_RECENT_FILES, this);
    recentFiles_->setObjectName("_RecentSceneFiles");
    recentFiles_->loadSettings();
    recentFiles_->setAutoSave(true);


    QMenu * m;
    QAction * a;

    // ######### SCENE MENU #########
    m = new QMenu(tr("Scene"), menuBar);
    menuBar->addMenu(m);

        m->addAction(a = new QAction(tr("New scene"), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(newScene()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Load scene ..."), menuBar));
        a->setShortcut(Qt::CTRL + Qt::Key_L);
        connect(a, SIGNAL(triggered()), this, SLOT(loadScene()));

        m->addAction(a = new QAction(tr("Load recent scene"), menuBar));
        a->setMenu( recentFiles_->createMenu() );
        connect(a->menu(), SIGNAL(triggered(QAction*)),
                this, SLOT(onRecentFile_(QAction*)));

        m->addAction(a = actionSaveScene_ = new QAction(tr("Save scene"), menuBar));
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, SIGNAL(triggered()), this, SLOT(saveScene()));

        m->addAction(a = new QAction(tr("Save scene as ..."), menuBar));
        a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        connect(a, SIGNAL(triggered()), this, SLOT(saveSceneAs()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Show scene description ..."), menuBar));
        a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_I);
        connect(a, SIGNAL(triggered()), this, SLOT(showSceneDesc()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Quit"), menuBar));
        a->setShortcut(Qt::ALT + Qt::Key_F4);
        connect(a, SIGNAL(triggered()), this, SLOT(quit()));

#ifndef MO_DISABLE_FRONT
    // ######### INTERFACE MENU #########
    m = new QMenu(tr("Interface"), menuBar);
    menuBar->addMenu(m);

        m->addAction(a = new QAction(tr("New interface"), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(newInterface()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Import ..."), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(loadInterface()));

        m->addAction(a = new QAction(tr("Import and insert..."), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(insertInterface()));

        m->addAction(a = new QAction(tr("Export as ..."), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(saveInterfaceAs()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Import presets ..."), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(loadInterfacePresets()));

        m->addAction(a = new QAction(tr("Export presets as ..."), menuBar));
        connect(a, SIGNAL(triggered()), this, SLOT(saveInterfacePresetsAs()));

        m->addSeparator();

        m->addAction(a = new QAction(tr("Edit mode"), menuBar));
        a->setCheckable(true);
        a->setChecked(frontScene_->isEditMode());
        a->setShortcut(Qt::ALT + Qt::Key_E);
        connect(a, SIGNAL(triggered(bool)), frontScene_, SLOT(setEditMode(bool)));
#endif

    // ######### EDIT MENU #########
    m = menuEdit_ = new QMenu(tr("Edit"), menuBar);
    menuBar->addMenu(m);
    // will be updated from other widgets


    // ######### RENDER MENU #########
    m = new QMenu(tr("Engine"), menuBar);
    menuBar->addMenu(m);

        // ------ transport sub-menu -------

        QMenu* sub;
        sub = new QMenu(tr("Transport"), menuBar);
        m->addMenu(sub);

            QActionGroup * ag = new QActionGroup(menuBar);
            sub->addAction(a = new QAction(tr("Start"), menuBar));
            ag->addAction(a);
            a->setShortcut(Qt::Key_F7);
            a->setCheckable(true);
            connect(a, SIGNAL(triggered()), this, SLOT(start()));

            sub->addAction(a = new QAction(tr("Stop"), menuBar));
            ag->addAction(a);
            a->setShortcut(Qt::Key_F8);
            a->setCheckable(true);
            a->setChecked(true);
            connect(a, SIGNAL(triggered()), this, SLOT(stop()));

            sub->addAction(a = new QAction(tr("Back %1 sec").arg(timeStep1_), menuBar));
            a->setShortcut(Qt::SHIFT + Qt::Key_Left);
            connect(a, &QAction::triggered, [=](){ moveTime(-timeStep1_); });

            sub->addAction(a = new QAction(tr("Back %1 sec").arg(timeStep2_), menuBar));
            a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Left);
            connect(a, &QAction::triggered, [=](){ moveTime(-timeStep2_); });

            sub->addAction(a = new QAction(tr("Forward %1 sec").arg(timeStep1_), menuBar));
            a->setShortcut(Qt::SHIFT + Qt::Key_Right);
            connect(a, &QAction::triggered, [=](){ moveTime(timeStep1_); });

            sub->addAction(a = new QAction(tr("Forward %1 sec").arg(timeStep2_), menuBar));
            a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Right);
            connect(a, &QAction::triggered, [=](){ moveTime(timeStep2_); });

        m->addAction(a = new QAction(tr("Run scripts"), menuBar));
        a->setShortcut(Qt::Key_F9);
        connect(a, SIGNAL(triggered()), this, SLOT(runScripts()));

        m->addSeparator();

        // ##### RESOLUTION SUBMENU #####

        sub = new QMenu(tr("Resolution"), menuBar);
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
            a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_1);
            connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

            sub->addAction(a = aDrawLightSources_= new QAction(tr("Show lights"), sub));
            a->setCheckable(true);
            a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_2);
            connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

            sub->addAction(a = aDrawMicrophones_ = new QAction(tr("Show microphones"), sub));
            a->setCheckable(true);
            a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_3);
            connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

            sub->addAction(a = aDrawAudioSources_ = new QAction(tr("Show soundsources"), sub));
            a->setCheckable(true);
            a->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_4);
            connect(a, SIGNAL(triggered()), this, SLOT(updateDebugRender_()));

        m->addSeparator();


        // ##### PROJECTOR INDEX SUBMENU #####
        sub = menuProjectorIndex_ = new QMenu(tr("Projector index"), menuBar);
        m->addMenu(sub);
        // will be updated by onProjectionSettingsChanged_();

            connect(sub, &QMenu::triggered, [=](QAction*a)
            {
                settings()->setClientIndex(a->data().toInt());
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
            closeAudio();
            AudioDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Midi settings"), m);
        m->addAction(a);
        a->setIcon(QIcon(":/icon/midi.png"));
        connect(a, &QAction::triggered, [=]()
        {
            closeAudio();
            MidiSettingsDialog diag;
            diag.exec();
        });

#ifndef MO_DISABLE_LADSPA
        a = new QAction(tr("Audio Plugins"), m);
        m->addAction(a);
        a->setIcon(QIcon(":/icon/obj_note.png"));
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new AudioPluginDialog(window_);
            diag->setAttribute(Qt::WA_DeleteOnClose, true);
            diag->show();
        });
#endif

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

        a = new QAction(tr("OpenGL info"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            QMessageBox::information(window_, tr("OpenGL info"),
                                     GL::Properties::staticInstance().toString());
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

        a = new QAction(tr("Audio-filter tester"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            AudioFilterDialog diag;
            diag.exec();
        });

#ifndef MO_HAMBURG
        a = new QAction(tr("Timeline editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            TimelineEditDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Wave Tracer"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            closeAudio();
            WaveTracerDialog * diag = new WaveTracerDialog(window_);
            connect(diag, SIGNAL(finished(int)), diag, SLOT(deleteLater()));
            diag->show();
        });

        a = new QAction(tr("SSW Import"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new SswImporter(window_);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            diag->setRootObject(scene_);
            diag->show();
        });
#endif


#ifdef MO_ENABLE_PYTHON34
        a = new QAction(tr("Python Script"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new QDialog(application()->mainWindow());
            diag->setObjectName("_Python34Test");
            diag->setWindowTitle(tr("Python 3.4 script"));
            diag->setMinimumSize(320,320);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            settings()->restoreGeometry(diag);
            auto l = new QVBoxLayout(diag);

            auto script = new Python34Widget(diag);
            l->addWidget(script);

            script->setScriptText(settings()->value("tmp/Python34", "").toString());
            auto but = new QPushButton(tr("&Run"), diag);
            but->setShortcut(Qt::ALT + Qt::Key_Return);
            l->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                settings()->storeGeometry(diag);
                settings()->setValue("tmp/Python34", script->scriptText());
                script->executeScript();
            });
            diag->show();
        });
#endif

        a = new QAction(tr("Batch scene converter"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            SceneConvertDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Bulk file renamer"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            BulkRenameDialog diag;
            diag.exec();
        });

        a = new QAction(tr("Polygon to image converter"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new PolygonImageImporter(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->show();
        });

        a = new QAction(tr("Distance field image"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new DistanceFieldImage(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->show();
        });

        a = new QAction(tr("CSG Object Editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new CsgDialog(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->show();
        });

        a = new QAction(tr("Evolution"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto win = new EvolutionDialog(window_);
            win->setAttribute(Qt::WA_DeleteOnClose, true);
            win->show();
        });

        a = new QAction(tr("StyleSheet editor"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new TextEditDialog(settings()->styleSheet(), TT_APP_STYLESHEET, qApp->activeWindow());
            diag->setAttribute(Qt::WA_DeleteOnClose);
            diag->setModal(false);
            connect(diag, &TextEditDialog::textChanged, [=]()
            {
                settings()->setStyleSheet(diag->getText());
                application()->setStyleSheet(diag->getText());
            });
            diag->show();
        });


    // ####################### DEBUG MENU #####################

    m = new QMenu(tr("Debug"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("QObject inspector (mainwindow)"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto o = new QObjectInspector(window_, window_);
            o->setAttribute(Qt::WA_DeleteOnClose);
            o->show();
        });

        a = new QAction(tr("QObject inspector (application)"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto o = new QObjectInspector(application(), window_);
            o->setAttribute(Qt::WA_DeleteOnClose);
            o->show();
        });

        a = new QAction(tr("Image list"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new ImageListDialog(window_);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            diag->show();
        });

        m->addAction(a = new QAction(tr("Dump id names"), m));
        connect(a, SIGNAL(triggered()), SLOT(dumpIdNames_()));
#ifdef MO_GRAPH_DEBUG
        m->addAction( a = new QAction(tr("Dump object tree"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto tree = get_object_tree(scene_);
            tree->dumpTree(std::cout, "");
            delete tree;
        });
#endif
        m->addAction(a = new QAction(tr("Dump needed files"), m));
        connect(a, SIGNAL(triggered()), SLOT(dumpNeededFiles_()));
#ifndef MO_HAMBURG
        m->addAction(a = new QAction(tr("Test transformation speed (new)"), m));
        connect(a, &QAction::triggered, [this](){ testSceneTransform_(true); });

        m->addAction(a = new QAction(tr("Test transformation speed (old)"), m));
        connect(a, &QAction::triggered, [this](){ testSceneTransform_(false); });
#endif
        m->addAction(a = new QAction(tr("Test audio speed"), m));
        connect(a, &QAction::triggered, [this](){ testAudioSpeed(); });
#ifndef MO_DISABLE_EXP
        m->addAction(a = new QAction(tr("Export scene to povray"), m));
        connect(a, SIGNAL(triggered()), SLOT(exportPovray_()));
#endif
        m->addAction( a = new QAction(tr("Info window"), m) );
        connect(a, &QAction::triggered, [=]()
        {
            auto w = new InfoWindow(window_);
            w->showFullScreen();
        });

#ifndef MO_HAMBURG
        m->addAction( a = new QAction(tr("Dump modulation graph"), m) );
        connect(a, &QAction::triggered, [=]()
        {
#ifdef MO_GRAPH_DEBUG
            auto tree = get_object_tree(scene_);
            tree->dumpTree(std::cout);
#endif
            ObjectGraph graph;
            get_object_modulator_graph(graph, scene_);
#ifdef QT_DEBUG
            graph.dumpEdges(std::cout);
#endif
            QVector<Object*> linear;
            graph.makeLinear(std::inserter(linear, linear.begin()));
            std::cout << "linear: ";
            for (auto o : linear)
                std::cout << " " << o->idName();
            std::cout << std::endl;
        });
#endif

#ifndef MO_DISABLE_ANGELSCRIPT
        a = new QAction(tr("AngelScript test"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new QDialog(application()->mainWindow());
            diag->setObjectName("_AngelScriptTest");
            diag->setWindowTitle(tr("AngelScript test"));
            diag->setMinimumSize(580,540);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            settings()->restoreGeometry(diag);
            auto l = new QVBoxLayout(diag);

            auto script = new AngelScriptWidget(diag);
            registerDefaultAngelScript( script->scriptEngine() );
            registerAngelScript_rootObject( script->scriptEngine(), scene_, true );

            l->addWidget(script);
            script->setScriptText(settings()->value("tmp/AngelScript", exampleAngelScript()).toString());
            auto but = new QPushButton(tr("&Run"), diag);
            but->setShortcut(Qt::ALT + Qt::Key_Return);
            l->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                settings()->storeGeometry(diag);
                settings()->setValue("tmp/AngelScript", script->scriptText());
                script->executeScript();
            });
            diag->show();
        });
#ifndef MO_HAMBURG
        a = new QAction(tr("export AngelScript namespace"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            exportAngelScriptFunctions("./angelscript_export.xml");
        });
#endif
#endif


    // ######### VIEW MENU #########
    viewMenu_ = m = new QMenu("View", menuBar);
    menuBar->addMenu(viewMenu_);

        a = aGlWindowVisible_ = new QAction(tr("Output window"), m);
        a->setCheckable(true);
        a->setChecked(glManager_->isWindowVisible());
        m->addAction(a);
        connect(a, &QAction::triggered, [=](bool check)
        {
            glManager_->setWindowVisible(check);
        });

    // ######### HELP MENU #########
    m = new QMenu(tr("Help"), menuBar);
    menuBar->addMenu(m);

        a = new QAction(tr("Help index"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new HelpDialog(window_);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            diag->show();
        });

        a = new QAction(tr("Context help"), m);
        a->setShortcut(Qt::Key_F1);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            auto diag = new HelpDialog(currentHelpUrl(), window_);
            diag->setAttribute(Qt::WA_DeleteOnClose);
            diag->show();
        });

        m->addSeparator();

        a = new QAction(tr("About this software"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), application(), SLOT(aboutMO()));

        a = new QAction(tr("About Qt"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), application(), SLOT(aboutQt()));

        a = new QAction(tr("Changes"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), application(), SLOT(showChanges()));

        m->addSeparator();

        a = new QAction(tr("Export help (html)"), m);
        m->addAction(a);
        connect(a, SIGNAL(triggered()), this, SLOT(exportHelpHtml_()));

    // create projector-index-menu
    onProjectionSettingsChanged_();
}



void MainWidgetController::setScene_(Scene * s, const SceneSettings * set)
{
    MO_DEBUG_GUI("MainWidgetController::setScene_(" << s << ")");

    MO_ASSERT(s, "MainWidgetController::setScene_() with NULL scene");
    MO_ASSERT(s != scene_, "MainWidgetController::setScene_() with same scene");

    //if (renderEngine_)
    //    renderEngine_->release();

    objectTreeView_->setRootObject(nullptr);
    objectView_->setObject(nullptr);
    sequencer_->setNothing();

    if (audioEngine_)
        audioEngine_->stop();

    if (scene_)
    {
        glManager_->setScene(nullptr);
        //scene_->kill();
        scene_->releaseRef("MainWidgetController release scene");
    }

    scene_ = s;

    scene_->setObjectEditor(objectEditor_);
    scene_->setCurrentScene(scene_);

    /** @todo When to run startup scripts? */
    scene_->runScripts();

    // clear or init scene gui settings
    if (!set)
        sceneSettings_->clear();
    else
    {
        *sceneSettings_ = *set;
    }

    objectGraphView_->setGuiSettings(sceneSettings_);

#ifndef MO_DISABLE_FRONT
    // --- get the interface ---
    QString xml = scene_->frontSceneXml();
    if (xml.isEmpty())
    {
        // XXX We can use the non-signalling version here
        // because we are rebuilding the gui and scene anyway
        frontScene_->clear();
    }
    else
    {
        try
        {
            frontScene_->setXml(xml);
        }
        catch (const Exception& e)
        {
            QMessageBox::critical(window_, tr("Interface"),
                                  tr("Failed to load the interface\n%1").arg(e.what()));
            frontScene_->clear();
        }
    }
#endif

    // check for local filenames

    IO::FileList files;
    scene_->getNeededFiles(files);

    IO::fileManager().clear();
    IO::fileManager().addFilenames(files);
    IO::fileManager().acquireFiles();
#ifndef NDEBUG
    IO::fileManager().dumpStatus();
#endif
    // ---------- opengl stuff ----------

    MO_ASSERT(glManager_, "");

    // set current resolution
    if (scene_->doMatchOutputResolution())
        scene_->setResolution(outputSize_);

    // set projection settings
    updateSceneProjectionSettings_();

    // update render settings from MainWidgetController
    updateDebugRender_();

    // XXX debug
/*    if (!renderEngine_)
        renderEngine_ = new RenderEngine(this);
    renderEngine_->setScene(scene_, glManager_->renderer()->context(), MO_GFX_THREAD);
*/

    // create the udp stream
    // YYY serverEngine().getAudioOutStream();

    // connect to render window
    glManager_->setScene(scene_);
    connect(glManager_, &GL::Manager::cameraMatrixChanged, [=](const Mat4& m)
    {
        scene_->setFreeCameraMatrix(m);
    });


    // widgets -> scenetime
    connect(seqView_, SIGNAL(sceneTimeChanged(Double)),
            this, SLOT(setSceneTime(Double)));
    connect(sequencer_, SIGNAL(sceneTimeChanged(Double)),
            this, SLOT(setSceneTime(Double)));

    // scene changes
    connect(scene_->sceneSignals(), SIGNAL(numberOutputEnvelopesChanged(uint)),
            this, SLOT(updateNumberOutputEnvelopes_(uint)));
    connect(scene_->sceneSignals(), SIGNAL(sceneTimeChanged(Double)),
            this, SLOT(onSceneTimeChanged_(Double)));

    // update widgets

    connect(scene_->sceneSignals(),
            SIGNAL(parameterVisibilityChanged(MO::Parameter*)),
            objectView_, SLOT(updateParameterVisibility(MO::Parameter*)));

    objectTreeView_->setRootObject(scene_);
    objectGraphView_->setRootObject(scene_);
    objectView_->setObject(scene_);
    objectOutputView()->setObject(0);

    seqView_->setScene(scene_);
    seqView_->setNothing();

    sequencer_->setCurrentObject(scene_);
    clipView_->setScene(scene_);

#ifndef MO_DISABLE_FRONT
    frontItemEditor_->setItem(0);
    frontScene_->setRootObject(scene_);
    frontScene_->assignObjects();
    frontScene_->loadPreset("default");
#endif

    glManager_->render();

    updateSystemInfo_();
    updateResolutionActions_();

#ifndef MO_DISABLE_SERVER
    if (isServer())
        if (serverEngine().isRunning())
            serverEngine().sendScene(scene_);
#endif
}









void MainWidgetController::onWindowKeyPressed_(QKeyEvent * e)
{
#if 0
    // XXX How to pass the event to the window???
    // This does not trigger the actions
    QCoreApplication::postEvent(window_, new QKeyEvent(*e));

#else
    if (e->key() == Qt::Key_F7 && !isPlaying())
    {
        e->accept();
        start();
    }

    if (e->key() == Qt::Key_F8)
    {
        e->accept();
        stop();
    }

    if (e->key() == Qt::Key_F9)
    {
        e->accept();
        runScripts();
    }

    if ((e->key() >= Qt::Key_0 && e->key() <= Qt::Key_9)
        && e->modifiers() == 0)
    {
        e->accept();
        scene_->setFreeCameraIndex(e->key() - Qt::Key_0 - 1);
    }

    if (((e->modifiers() & Qt::SHIFT) || (e->modifiers() & Qt::CTRL))
            && (e->key() == Qt::Key_Left || e->key() == Qt::Key_Right))
    {
        Double ti = timeStep1_;
        if (e->modifiers() & Qt::CTRL)
            ti = timeStep2_;
        if (e->key() == Qt::Key_Left)
            ti = -ti;
        moveTime(ti);
    }

//    if (!e->isAccepted())
//        scene_->keyDown(e->key());
#endif
}


void MainWidgetController::onUpdateTimer_()
{
    //if (audioEngine_)
    //    transportWidget_->
    onSceneTimeChanged_(audioEngine_->second());
    updateOutputEnvelope_(audioEngine_->outputEnvelope());
}


void MainWidgetController::onObjectAdded_(Object * o)
{
    // copy scene-gui-settings from original object (if pasted)
    copySceneSettings_(o);

    // update clipview
    if (Clip * clip = dynamic_cast<Clip*>(o))
    {
        if (clip->clipContainer() == clipView_->clipContainer())
            clipView_->updateClip(clip);
    }

    updateSequenceView_(o);
    objectGraphView()->setFocusObject(o);
    objectView()->setObject(o);
    objectOutputView()->setObject(o);

    onSceneChanged_();
}

void MainWidgetController::onObjectDeleted_(const Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::onObjectDeleted_(" << (void*)o << ")");

    // update clipview
    clipView_->removeObject(o);

    // XXX refine this!
    objectView()->setObject(0);
    objectOutputView()->setObject(0);
    sequenceView()->setNothing();
    //sequencer()->setCurrentObject(0);

    onSceneChanged_();
}

void MainWidgetController::onObjectsDeleted_(const QList<Object*>& l)
{
    // update clipview
    for (auto o : l)
        clipView_->removeObject(o);

    // XXX refine this!
    objectView()->setObject(0);
    objectOutputView()->setObject(0);
    sequenceView()->setNothing();
    //sequencer()->setCurrentObject(0);

    onSceneChanged_();
}






void MainWidgetController::showClipView_(bool enable, Object * o)
{
    //if (enable)
    //    sequencer_->setVisible(false);
    clipView_->setVisible(enable);

    if (!enable)
    {
        emit modeChanged();
        return;
    }

    clipView_->selectObject(o);

    emit modeChanged();
}

void MainWidgetController::showSequencer_(bool enable, Object * o)
{
    sequencer_->setVisible(enable);
    //clipView_->setVisible(!enable);
    emit modeChanged();

    if (!enable)
    {
        emit modeChanged();
        return;
    }

    sequencer_->setCurrentObject(o->rootObject() ? o->rootObject() : o);
    emit modeChanged();
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

    emit modeChanged();
}

void MainWidgetController::showValueFloat_(ValueFloatInterface * iface)
{
    seqView_->setValueFloat(iface);
    seqView_->setVisible(true);

    emit modeChanged();
}

/** Shows or hides the sequence view and selects a sequence to display */
void MainWidgetController::updateSequenceView_(Object * o)
{
    if (!o)
    {
        MO_WARNING("NULL in updateSequenceView_()");
        return;
    }

    // show the clicked-on sequence
    if (o->isSequence())
        showSequence_(true, static_cast<Sequence*>(o));

    else
    if (auto iface = dynamic_cast<ValueFloatInterface*>(o))
        showValueFloat_(iface);

    else
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

    if (o)
        setHelpUrl("_object_" + o->className());

    // update object editor
    objectView_->setObject(o);
    objectOutputView()->setObject(o);
    objectGraphView_->setFocusObject(o);

    if (!o)
    {
        showSequencer_(false);
        showSequence_(false);
        return;
    }

    // update sequence view
    updateSequenceView_(o);

    // update clipview
    if (o->isClip() || o->isClipController() ||
              o->findParentObject(Object::T_CLIP_CONTROLLER))
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

    if (o)
        setHelpUrl("_object_" + o->className());

    // update object editor
    objectView_->setObject(o);
    objectOutputView()->setObject(o);
    objectTreeView_->selectObject(o);

    if (!o)
    {
        showSequencer_(false);
        showSequence_(false);
        return;
    }

    // update sequence view
    updateSequenceView_(o);

    // update clipview
    if (o->isClip() || o->isClipController() ||
              o->findParentObject(Object::T_CLIP_CONTROLLER))
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

    if (o)
        setHelpUrl("_object_" + o->className());

    // update object editor
    objectView_->setObject(o);

    // jump to clip in graph view
    objectGraphView()->setFocusObject(o);
    objectView()->selectObject(o);
    // show sequence
    updateSequenceView_(o);
}


void MainWidgetController::onObjectSelectedObjectView_(Object * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedObjectView_(" << o << ")");

    if (o)
        setHelpUrl("_object_" + o->className());

    // update sequence view
    updateSequenceView_(o);

    // jump to modulator in tree view
    objectGraphView()->setFocusObject(o);

    // update clipview
    if (o && (o->isClip() || o->isClipController() ||
              o->findParentObject(Object::T_CLIP_CONTROLLER)))
    {
        showClipView_(true, o);
    }

    objectOutputView()->setObject(o);

/*
    // update sequencer
    else
    {
        showSequencer_(true, o);
    }
    */
}


void MainWidgetController::onObjectSelectedSequencer_(Sequence * o)
{
    MO_DEBUG_GUI("MainWidgetController::objectSelectedSequencer_(" << o << ")");

    if (o)
        setHelpUrl("_object_" + o->className());

    // update object editor
    objectView_->setObject(o);

    // jump to sequence in tree view
    objectGraphView()->setFocusObject(o);
    objectView()->selectObject(o);
    objectOutputView()->setObject(o);
    // show sequence
    updateSequenceView_(o);
}

void MainWidgetController::onSequenceClicked_()
{
    Object * obj = seqView_->sequence();
    if (!obj)
        return;
        //obj = seqView_->valueFloat();

    // update objectview if not already
    if (objectView_->object() != obj)
        objectView_->setObject(obj);

    objectGraphView()->setFocusObject(obj);
    objectView()->selectObject(obj);
    objectOutputView()->setObject(obj);
}

void MainWidgetController::onParamVisChanged_()
{
#ifndef MO_DISABLE_FRONT
    // rebuild interface items
    frontItemEditor_->setItem(0);
    if (scene_)
        frontScene_->setRootObject(scene_);
#endif
}

void MainWidgetController::onParamChanged_()
{
    // Most stuff is handled by the views themselves

    objectOutputView()->updateObject();
}

void MainWidgetController::onParamsChanged_(Object*o)
{
    if (dynamic_cast<ValueFloatInterface*>(o))
        onValueFloatChanged_();
    objectOutputView()->updateObject();
}



void MainWidgetController::onSceneChanged_()
{
    if (!sceneNotSaved_)
    {
        sceneNotSaved_ = true;
        updateWindowTitle_();
    }
}

void MainWidgetController::onValueFloatChanged_()
{
    if (seqView_->isVisible())
        seqView_->update();
}

void MainWidgetController::onUiEditModeChanged_(bool /*isEdit*/)
{
    /* should show/hide the dock widget for the item properties */
}

void MainWidgetController::onSceneTimeChanged_(Double time)
{
    if (seqView_->isVisible())
        seqView_->setSceneTime(time);
    if (sequencer_->isVisible())
        sequencer_->setSceneTime(time);

    Double fps = glManager_->messuredFps();
    transportWidget_->setSceneTime(time, fps);
}

void MainWidgetController::setEditActions_(const QObject *, const ActionList &actions)
{
#ifndef __APPLE__
    menuEdit_->clear();
    menuEdit_->addActions(actions);
#endif
}

void MainWidgetController::testSceneTransform_(bool newVersion)
{
    QTime t;

    int num = 50000000;
    int i = 0;
    int e = 0;

    if (!newVersion)
    {
        t.start();
        for (; i < num && e <= 1000;)
        {
            for (int j=0; j<1000; ++j, ++i)
                scene_->calculateSceneTransform(
                            RenderTime(scene_->sampleRateInv() * (i*1000+j), MO_GFX_THREAD));

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
        const auto bufsize = 128;//scene_->bufferSize(MO_AUDIO_THREAD);

        ObjectDspPath dsp;
        dsp.createPath(scene_, AUDIO::Configuration(scene_->sampleRate(), bufsize, 0, 2), MO_AUDIO_THREAD);

        t.start();
        for (; i < num && e <= 1000; )
        {
            for (int j=0; j<20; ++j, i += bufsize)
                dsp.calcTransformations(i*1000+j);

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



void MainWidgetController::testAudioSpeed()
{
    AUDIO::Configuration conf(scene_->sampleRate(),
                              128,//scene_->bufferSize(MO_AUDIO_THREAD),
                              2, 2);

    AudioEngine engine;

    engine.setScene(scene_, conf, MO_AUDIO_THREAD);

    // temp in/out buffers
    std::vector<F32>
            fakeIns(conf.sampleRate() * conf.bufferSize() * 2),
            fakeOuts(conf.sampleRate() * conf.bufferSize() * 2);

    int     num = 50000000,
            // number of dsp steps before messuring time
            // to not trigger the clock at every block
            // and in case the clock is too course
            // [..which is true for QTime anyway]
            numInFrames = std::max(1u, 30000 / conf.bufferSize()),

            i = 0, e = 0;

    QTime t;
    t.start();
    for (; i < num && e <= 1000; )
    {
        for (int j=0; j<numInFrames; ++j, i += conf.bufferSize())
            engine.process(&fakeIns[0], &fakeOuts[0]);

        e = t.elapsed();
    }
    num = i;
    const Double elapsed = (Double)e / 1000.0;

    QMessageBox::information(window_, tr("Scene audio benchmark"),
        tr("<html>Processed %1 samples of audio/spatial dsp stuff"
           "<br/>which took %2 seconds."
           "<br/>This is %3 milli-secs per sample"
           "<br/>%4 ms per dsp block(%5)"
           "<br/>and <b>%6</b> samples per second"
           "<br/>which is <b>%7</b> times realtime (@%8hz)</html>")
                             .arg(num)
                             .arg(elapsed)
                             .arg((elapsed*1000)/num)
                             .arg((elapsed*1000)/num*conf.bufferSize())
                             .arg(conf.bufferSize())
                             .arg((int)((Double)num/elapsed))
                             .arg((Double)num / elapsed / conf.sampleRate())
                             .arg(conf.sampleRate())
           );
}


void MainWidgetController::showSceneDesc()
{
    if (!scene_)
        return;

    auto diag = new SceneDescDialog(window_);
    diag->setAttribute(Qt::WA_DeleteOnClose, true);
    diag->setAttribute(Qt::WA_AlwaysStackOnTop, true);
    diag->setText(scene_->sceneDesc());
    diag->setShowOnStart(scene_->showSceneDesc());
    connect(diag, &SceneDescDialog::accepted, [=]()
    {
        if (scene_->sceneDesc() != diag->text()
            || scene_->showSceneDesc() != diag->showOnStart())
        {
            scene_->setSceneDesc(diag->text(), diag->showOnStart());
            onSceneChanged_();
        }
    });

    diag->show();
    diag->raise();
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

void MainWidgetController::copySceneSettings_(Object *)
{
    /*YYY if (o->idName() != o->originalIdName())
        sceneSettings_->copySettings(o->idName(), o->originalIdName());

    for (auto c : o->childObjects())
        copySceneSettings_(c);*/
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
    return audioEngine_ && audioEngine_->isPlayback();
}

bool MainWidgetController::isPlaying() const
{
    return audioEngine_ && audioEngine_->isPlayback()
                        && !audioEngine_->isPause();
}

void MainWidgetController::quit()
{
    window_->close();
}

void MainWidgetController::onGlWindowVisibleChanged_(bool v)
{
    if (aGlWindowVisible_)
        aGlWindowVisible_->setChecked(v);
}

void MainWidgetController::start()
{
    //scene_->start();

    // prepare audio engine
    if (!audioEngine_)
        audioEngine_ = new LiveAudioEngine(this);
    if (audioEngine_->scene() != scene_)
        audioEngine_->setScene(scene_, MO_AUDIO_THREAD);

    CurrentTime::setAudioEngine(audioEngine_);

    // update audio config for clients
#ifndef MO_DISABLE_SERVER
    if (isServer())
        if (serverEngine().isRunning())
            serverEngine().sendAudioConfig( audioEngine_->config() );
#endif

    // update sampling rate
    if (scene_->sampleRate() != audioEngine_->config().sampleRate())
        scene_->setSceneSampleRate(audioEngine_->config().sampleRate());

    // audio input/output channels may have changed
    // XXX not smart - but reliable
    objectGraphView()->setRootObject(scene_);
    // also tell the envelope display
    updateNumberOutputEnvelopes_(audioEngine_->config().numChannelsOut());

    // start engine
    if (audioEngine_->start())
    {
        // start rythmic gui updates
        updateTimer_->start();

        glManager_->startAnimate();
#ifndef MO_DISABLE_FRONT
        frontScene_->startAnimate();
#endif
        // XXX especially update CurrentTime (for clients)
        // Scene::sceneTime() is not really used
        scene_->setSceneTime(audioEngine_->second());

#ifndef MO_DISABLE_SERVER
        if (isServer())
            if (serverEngine().isRunning())
                serverEngine().setScenePlaying(true);
#endif
    }

    scene_->setPlaying(true);
    transportWidget_->setPlayback(true);
}

void MainWidgetController::stop()
{
    glManager_->stopAnimate();
#ifndef MO_DISABLE_FRONT
    frontScene_->stopAnimate();
#endif
    updateTimer_->stop();
    scene_->setPlaying(false);

    if (audioEngine_)
    {
        if (audioEngine_->isPlayback() && !audioEngine_->isPause())
            audioEngine_->pause(true);
        else
        {
            Double time = getPrevLocatorTime_(audioEngine_->second());
            audioEngine_->seek(time);
            // XXX hacky
            //onSceneTimeChanged_(0.0);
            scene_->setSceneTime(time);
            scene_->render();
        }
    }
    else
    {
        scene_->setSceneTime(0.0);
        scene_->render();
    }

#ifndef MO_DISABLE_SERVER
    if (serverEngine().isRunning())
        serverEngine().setScenePlaying(false);
#endif

    transportWidget_->setPlayback(false);
}

void MainWidgetController::closeAudio()
{
    if (audioEngine_)
        audioEngine_->closeAudioDevice();
}

Double MainWidgetController::getPrevLocatorTime_(Double time)
{
    if (scene_->locators().empty())
        return 0.;
#if 0
    MO_PRINT("--");
    for (auto i : scene_->locators())
        MO_PRINT(i.time << " " << i.id);
#endif
    auto i = scene_->locators().end();
    while (true)
    {
        --i;
        if (i->time < time)
            return i->time;
        if (i == scene_->locators().begin())
            return 0.;
    }
}

void MainWidgetController::moveTime(Double sec)
{
    Double time = std::max(Double(0),
                    sec + (isPlayback()
                            ? (audioEngine_ ? audioEngine_->second() : 0.0)
                            : (scene_ ? scene_->sceneTime() : 0.0)) );
    setSceneTime(time);
}

void MainWidgetController::setSceneTime(Double time)
{
    // does CurrentTime as well
    if (scene_)
        scene_->setSceneTime(time);
    // that's our actual clock
    if (audioEngine_)
        audioEngine_->seek(time);

    if (scene_)
        scene_->render();
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


void MainWidgetController::runScripts()
{
    if (scene_)
        scene_->runScripts();
}

void MainWidgetController::newScene()
{
    if (isPlayback())
        stop();

    if (!isOkayToChangeScene())
        return;

    // create a new scene with a group inside
    auto s = ObjectFactory::createSceneObject();
    auto group = ObjectFactory::createObject("Group");
    s->addObject(s, group);
    group->releaseRef("finished add-to-scene");

    setScene_( s );
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
    QString fn;
    fn = IO::Files::filename(IO::FT_SCENE);

    try
    {
        // load previous project ?
        if (fn.isEmpty())
            newScene();
        else
        {
            // clear filename temporarily (against continous crash-on-load)
            IO::Files::setFilename(IO::FT_SCENE, "");
            settings()->sync();
            loadScene_(fn);
            // and set back
            IO::Files::setFilename(IO::FT_SCENE, fn);
            // show scene description
            if (scene_ && scene_->showSceneDesc())
                showSceneDesc();
        }
    }
    catch (IoException& e)
    {
        MO_WARNING("IO error on load:\n" << e.what());
        IO::Files::setFilename(IO::FT_SCENE, fn);
        newScene();
    }
}

void MainWidgetController::onRecentFile_(QAction * a)
{
    QString fn = a->data().toString();
    if (!fn.isEmpty())
        loadScene(fn);
}

QString MainWidgetController::getSceneSaveFilename_()
{
    /** @todo fix that update problem with gl window
        means: leave idle time for other gui parts. */
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

void MainWidgetController::loadScene(const QString& fn)
{
    if (isPlayback())
        stop();

    if (!isOkayToChangeScene())
        return;

    if (loadScene_(fn))
    {
        IO::Files::setFilename(IO::FT_SCENE, fn);

        if (scene_ && scene_->showSceneDesc())
            showSceneDesc();
    }
}

void MainWidgetController::loadScene()
{
    if (isPlayback())
        stop();

    if (!isOkayToChangeScene())
        return;

    QString fn = IO::Files::getOpenFileName(IO::FT_SCENE, window_);

    if (loadScene_(fn))
    {
        if (scene_ && scene_->showSceneDesc())
            showSceneDesc();
    }
}

bool MainWidgetController::loadScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        // read scene file
        Scene * scene;
        try
        {
            scene = ObjectFactory::loadScene(fn);
            //scene->setLazyFlag(true);
            recentFiles_->addFilename(fn);
        }
        catch (Exception& e)
        {
            QMessageBox::critical(window_, tr("load scene failed"),
                                  tr("Could not open project '%1'\n%2").arg(fn).arg(e.what()));

            statusBar()->showMessage(tr("Could not open project '%1'").arg(fn));

            newScene();
            return false;
        }

        if (!scene)
        {
            QMessageBox::critical(window_, tr("load scene failed"),
                                  tr("Something went wrong opening the project\n'%1'")
                                  .arg(fn));
            statusBar()->showMessage(tr("Could not open project '%1'").arg(fn));
            newScene();
            return false;
        }

        QString err = scene->getIoLoadErrors();
        if (!err.isEmpty())
        {
            QMessageBox::warning(window_, tr("load scene warning"),
                                 tr("There was something odd when loading the scene\n%1\n%2\n\n"
                                    "Make sure you know what you are doing before you proceed. "
                                    "Especially saving the scene might be bad.")
                                 .arg(fn)
                                 .arg(err));
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
    {
        statusBar()->showMessage(tr("loading cancelled"), statusMessageTimeout_);
        return false;
    }
    return true;
}

bool MainWidgetController::saveScene_(const QString &fn)
{
    if (!fn.isEmpty())
    {
        try
        {
#ifndef MO_DISABLE_FRONT
            // always store default preset
            frontScene_->storePreset("default");
            scene_->setFrontScene(frontScene_);
#endif
            // actually save the scene
            ObjectFactory::saveScene(fn, scene_);
            recentFiles_->addFilename(fn);
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

#ifndef MO_DISABLE_FRONT
void MainWidgetController::saveInterfaceAs()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_INTERFACE_XML, window_);
    if (fn.isEmpty())
        return;

    try
    {
        frontScene_->saveXml(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(window_, tr("interface i/o"),
                              tr("Could not save the interface xml\n'%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
}

void MainWidgetController::newInterface()
{
    frontScene_->clearInterface();
}

void MainWidgetController::loadInterface()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_INTERFACE_XML, window_);
    if (fn.isEmpty())
        return;

    try
    {
        frontScene_->loadXml(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(window_, tr("interface i/o"),
                              tr("Could not load the interface xml\n'%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
}

void MainWidgetController::insertInterface()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_INTERFACE_XML, window_);
    if (fn.isEmpty())
        return;

    try
    {
        frontScene_->insertXml(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(window_, tr("interface i/o"),
                              tr("Could not load the interface xml\n'%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
}

void MainWidgetController::saveInterfacePresetsAs()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_INTERFACE_PRESET, window_);
    if (fn.isEmpty())
        return;

    frontScene_->exportPresets(fn);
}

void MainWidgetController::loadInterfacePresets()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_INTERFACE_PRESET, window_);
    if (fn.isEmpty())
        return;

    frontScene_->importPresets(fn);
}
#endif


void MainWidgetController::renderToDisk()
{
    bool isTemp = false;
    QString fn = currentSceneFilename_;

    if (sceneNotSaved_)
    {
        fn = IO::Files::getTempFilename(".mo3");
        if (fn.isEmpty())
        {
            QMessageBox::critical(window_, tr("render to disk"),
                tr("Could not create a temporary file.\n"
                   "Please save the scene and try again."));
            return;
        }
        isTemp = true;

        try
        {
#ifndef MO_DISABLE_FRONT
            // always store default preset
            frontScene_->storePreset("default");
            scene_->setFrontScene(frontScene_);
#endif
            // actually save the scene
            ObjectFactory::saveScene(fn, scene_);
        }
        catch (const Exception & e)
        {
            QMessageBox::critical(window_, tr("render to disk"),
                tr("Could not save a temporary scene file.\n%1\n"
                   "Please save the scene and try again.").arg(e.what()));
            QFile::remove(fn);
            return;
        }

    }
    auto diag = new RenderDialog(fn, window_);
    diag->exec();

    if (isTemp)
        QFile::remove(fn);
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

    const ProjectionSystemSettings& set = settings()->getDefaultProjectionSettings();
    for (uint i=0; i<set.numProjectors(); ++i)
    {
        auto a = new QAction(menuProjectorIndex_);
        a->setText(QString("%1 - %2")
                   .arg(i+1)
                   .arg(set.projectorSettings(i).name()));
        a->setData(i);
        a->setCheckable(true);
        a->setChecked(settings()->clientIndex() == (int)i);

        menuProjectorIndex_->addAction(a);
        g->addAction(a);
    }

}

void MainWidgetController::updateSceneProjectionSettings_()
{
    if (!scene_)
        return;

    scene_->setProjectionSettings(settings()->getDefaultProjectionSettings());
    scene_->setProjectorIndex(settings()->clientIndex());
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
    QString fn = IO::Files::getDirectory(IO::FT_HELP_EXPORT);
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
