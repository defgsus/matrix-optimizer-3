/** @file mainwidgetcontroller.h

    @brief Controller for all main edit widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 17.10.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
#define MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H

#include <QObject>
#include <QSize>
#include <QProgressBar>
#include <QMap>

#include "object/Object_fwd.h"
#include "types/float.h"
#include "tool/ProgressInfo.h"

class QStatusBar;
class QLabel;
class QMenuBar;
class QTimer;
class QAction;
class QKeyEvent;
class QMainWindow;
class QMenu;

namespace MO {
class LiveAudioEngine;
class RenderEngine;
class ValueFloatInterface;
class ActionList;
namespace GL { class Window; class Context; class Manager; }
namespace GUI {

class QObjectInspector;
class ObjectView;
#ifndef MO_DISABLE_TREE
class ObjectTreeView;
#endif
class SequenceView;
class Sequencer;
class SceneSettings;
class TransportWidget;
class ServerView;
class ClipView;
class ObjectGraphView;
#ifndef MO_DISABLE_FRONT
class FrontScene;
class FrontView;
class FrontItemEditor;
#endif
class RecentFiles;
class ObjectOutputView;
class AssetBrowser;
class OscView;

/** Top-Level widget container and gui logic. */
class MainWidgetController : public QObject
{
    Q_OBJECT
public:

    explicit MainWidgetController(QMainWindow * window);
    ~MainWidgetController();

    // --------------- getter ------------------

    bool isOkayToChangeScene();
    /** True when audio engine is running, not necessarily playing */
    bool isPlayback() const;
    /** True when audio engine is running AND playing */
    bool isPlaying() const;

    Scene * scene() const { return scene_; }
    SceneSettings * sceneSettings() const { return sceneSettings_; }
    GL::Manager * glManager() const { return glManager_; }
    GL::Window * glWindow() const { return glWindow_; }
    ObjectTreeView* objectTreeView() const { return objectTreeView_; }
    ObjectView * objectView() const { return objectView_; }
    ObjectEditor * objectEditor() const { return objectEditor_; }
    ObjectGraphView * objectGraphView() const { return objectGraphView_; }
    Sequencer * sequencer() const { return sequencer_; }
    ClipView * clipView() const { return clipView_; }
#ifndef MO_DISABLE_FRONT
    FrontView * frontView() const { return frontView_; }
    FrontItemEditor * frontItemEditor() const { return frontItemEditor_; }
#endif
    SequenceView * sequenceView() const { return seqView_; }
    ObjectOutputView * objectOutputView() const { return objectOutputView_; }
    TransportWidget * transportWidget() const { return transportWidget_; }
    AssetBrowser * assetBrowser() const { return assetBrowser_; }
    //QObjectInspector * objectInspector() const { return qobjectInspector_; }
    ServerView * serverView() const { return serverView_; }
    OscView * oscView() const { return oscView_; }
    QStatusBar * statusBar() const { return statusBar_; }
    QMenu * viewMenu() const { return viewMenu_; }

    void createMainMenu(QMenuBar * menuBar);

    // --------------- setter ------------------

    // ------------- signals -------------------

signals:

    void windowTitle(const QString& title);

    /** Widgets might have appeared or disappeared.
        XXX Not used yet. */
    void modeChanged();

    // ------------- actions -------------------

public slots:

    void quit();

    // --- transport ---
    void start();
    void stop();
    void moveTime(Double sec);
    /** Sets CurrentTime and audio engine's time at once */
    void setSceneTime(Double time);
    /** Closes the audio device */
    void closeAudio();

    /** Loads last or creates new */
    void initScene();

    /** @{ */ /** These functions all appropriately call isOkayToChangeScene() */
    /** Saves current scene, overwrites or asks for name if not given */
    bool saveScene();
    /** Saves current scene as new file */
    void saveSceneAs();
    /** Loads a new scene via file dialog */
    void loadScene();
    /** Loads the given scene filename */
    void loadScene(const QString& fn);
    /** Creates a new, empty scene */
    void newScene();
    /** Shows the scene description dialog */
    void showSceneDesc();
    /** @} */

    /** Executes all scripts in the scene */
    void runScripts();

#ifndef MO_DISABLE_FRONT
    void newInterface();
    void loadInterface();
    void insertInterface();
    void saveInterfaceAs();
    void loadInterfacePresets();
    void saveInterfacePresetsAs();
#endif

    void renderToDisk();

    void testAudioSpeed();

    // --------- private slots -----------------

private slots:

    void setEditActions_(const QObject * sender, const ActionList& actions);
    void testSceneTransform_(bool newVersion);
    void updateSystemInfo_();
    void updateDebugRender_();

    void onGlWindowVisibleChanged_(bool);

    void onObjectSelectedTree_(MO::Object*);
    void onObjectSelectedGraphView_(MO::Object*);
    void onObjectSelectedObjectView_(MO::Object*);
    void onObjectSelectedClipView_(MO::Object*);
    void onObjectSelectedSequencer_(MO::Sequence*);
    void onSequenceClicked_();

    void onObjectNameChanged_(MO::Object*);
    void onObjectAdded_(MO::Object*);
    void onObjectDeleted_(const MO::Object*);
    void onObjectsDeleted_(const QList<MO::Object*>&);
    void onParamVisChanged_();
    void onParamChanged_();
    void onParamsChanged_(MO::Object*);
    void onValueFloatChanged_();
    /** To trigger sceneNotSaved_ */
    void onSceneChanged_();
    void onSceneTimeChanged_(Double time);
    void onUiEditModeChanged_(bool isEdit);

    void onRecentFile_(QAction*);
    void onProjectionSettingsChanged_();
    void updateSceneProjectionSettings_();

    void onOutputSizeChanged_(const QSize&);
    void onProgress(const MO::ProgressInfo&);

    void dumpIdNames_();
    void dumpNeededFiles_();
    void exportPovray_();
    void exportHelpHtml_();

    void showClipView_(bool enable, Object * = 0);
    void showSequence_(bool enable, Sequence * = 0);
    void showSequencer_(bool enable, Object * = 0);
    void showValueFloat_(ValueFloatInterface * iface);

    void updateWidgetsActivity_();

    void updateNumberOutputEnvelopes_(uint);
    void updateOutputEnvelope_(const F32*);

    void onWindowKeyPressed_(QKeyEvent*);

    void onResolutionOutput_();
    void onResolutionCustom_();
    void onResolutionPredefined_(QAction*);

    void onUpdateTimer_();

    // -- network --

    void sendSceneToClients_();

private:

    void createObjects_();
    void setScene_(Scene *, const SceneSettings *set = 0);

    void updateSequenceView_(Object *o);
    void updateWindowTitle_();
    QString getSceneSaveFilename_();
    bool saveScene_(const QString& fn);
    bool loadScene_(const QString& fn);

    void copySceneSettings_(Object * o);
    void setPredefinedResolution_(int index);
    void updateResolutionActions_();
    void updateProgress_();

    void populateViewPresetMenu_();
    void setViewPreset_(const QString& name);
    void saveViewPreset_();

    Double getPrevLocatorTime_(Double time);

    QMainWindow * window_;
    Scene * scene_;

    QTimer * updateTimer_;

    QSize outputSize_;

    LiveAudioEngine * audioEngine_;
    //RenderEngine * renderEngine_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    ObjectEditor * objectEditor_;

    SceneSettings * sceneSettings_;

    ObjectTreeView* objectTreeView_;
    ObjectView * objectView_;
    ObjectGraphView * objectGraphView_;
    Sequencer * sequencer_;
    ClipView * clipView_;
    SequenceView * seqView_;
#ifndef MO_DISABLE_FRONT
    FrontScene * frontScene_;
    FrontView * frontView_;
    FrontItemEditor * frontItemEditor_;
#endif
    ObjectOutputView * objectOutputView_;
    AssetBrowser * assetBrowser_;
    TransportWidget * transportWidget_;
    OscView * oscView_;

    bool isVisibleSequencer_,
         isVisibleClipView_,
         isVisibleSeqView_;

    ServerView * serverView_;

    QStatusBar * statusBar_;
    QLabel * sysInfoLabel_;
    QTimer * sysInfoTimer_;

    QString currentSceneFilename_;

    RecentFiles * recentFiles_;

    QMenu * menuEdit_, * menuResolutions_,
          * menuProjectorIndex_,
          * viewMenu_, * viewPresetMenu_;

    QAction * actionSaveScene_,
            * aDrawLightSources_,
            * aDrawAudioSources_,
            * aDrawCameras_,
            * aDrawMicrophones_,
            * aResolutionOutput_,
            * aResolutionCustom_,
            * aResolutionPredefined_,
            * aGlWindowVisible_;

    bool sceneNotSaved_;

    QMap<QString, ProgressInfo> progressMap_;
    QMap<QString, QProgressBar*> progressBars_;

    // ----- config -------

    int statusMessageTimeout_;
    Double timeStep1_, timeStep2_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
