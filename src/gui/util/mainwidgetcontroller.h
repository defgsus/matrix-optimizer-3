/** @file mainwidgetcontroller.h

    @brief Controller for all main edit widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 17.10.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
#define MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H

#define MO_DISABLE_TREE

#include <QObject>
#include <QSize>

#include "object/object_fwd.h"
#include "types/float.h"

class QStatusBar;
class QLabel;
class QMenuBar;
class QTimer;
class QAction;
class QKeyEvent;
class QMainWindow;
class QMenu;

namespace MO {
namespace GL { class Window; class Context; class Manager; }
namespace GUI {

class QObjectInspector;
class ObjectView;
class ObjectTreeView;
class SequenceView;
class Sequencer;
class SceneSettings;
class TransportWidget;
class ServerDialog;
class ClipView;
class TestThread;
class ObjectGraphView;

class MainWidgetController : public QObject
{
    Q_OBJECT
public:
    explicit MainWidgetController(QMainWindow * window);
    ~MainWidgetController();

    // --------------- getter ------------------

    bool isOkayToChangeScene();
    bool isPlayback() const;

    Scene * scene() const { return scene_; }
    SceneSettings * sceneSettings() const { return sceneSettings_; }
    GL::Manager * glManager() const { return glManager_; }
    GL::Window * glWindow() const { return glWindow_; }
    ObjectView * objectView() const { return objectView_; }
#ifndef MO_DISABLE_TREE
    ObjectTreeView * objectTreeView() const { return objectTreeView_; }
    ObjectTreeModel * objectTreeModel() const { return objectTreeModel_; }
#endif
    ObjectGraphView * objectGraphView() const { return objectGraphView_; }
    Sequencer * sequencer() const { return sequencer_; }
    ClipView * clipView() const { return clipView_; }
    SequenceView * sequenceView() const { return seqView_; }
    TransportWidget * transportWidget() const { return transportWidget_; }
    QObjectInspector * objectInspector() const { return qobjectInspector_; }
    ServerDialog * serverDialog() const { return serverDialog_; }
    QStatusBar * statusBar() const { return statusBar_; }

    void createMainMenu(QMenuBar * menuBar);

    // --------------- setter ------------------

    // ------------- signals -------------------

signals:

    void windowTitle(const QString& title);

    // ------------- actions -------------------

public slots:

    void start();
    void stop();

    /** Loads last or creates new */
    void initScene();

    bool saveScene();
    void saveSceneAs();
    void loadScene();
    //void loadScene(const QString& fn);
    void newScene();

    void renderToDisk();

    // --------- private slots -----------------

private slots:

    void setEditActions_(const QObject * sender, QList<QAction*> actions);
    void testSceneTransform_();
    void updateSystemInfo_();
    void updateDebugRender_();

    void onObjectSelectedTree_(MO::Object*);
    void onObjectSelectedGraphView_(MO::Object*);
    void onObjectSelectedObjectView_(MO::Object*);
    void onObjectSelectedClipView_(MO::Object*);
    void onObjectSelectedSequencer_(MO::Sequence*);
    void onSequenceClicked_();

    void onObjectNameChanged_(MO::Object*);
    void onObjectAdded_(MO::Object*);
    void onObjectDeleted_(const MO::Object*);
    void onTreeChanged_();
    /** To trigger sceneNotSaved_ */
    void onSceneChanged_();

    void onProjectionSettingsChanged_();
    void updateSceneProjectionSettings_();

    void onOutputSizeChanged_(const QSize&);

    void resetTreeModel_();
    void runTestThread_();
    void dumpIdNames_();
    void dumpNeededFiles_();
    void exportPovray_();
    void exportHelpHtml_();

    void showClipView_(bool enable, Object * = 0);
    void showSequence_(bool enable, Sequence * = 0);
    void showSequencer_(bool enable, Object * = 0);

    void updateWidgetsActivity_();

    void updateNumberOutputEnvelopes_(uint);
    void updateOutputEnvelope_(const F32*);

    void onWindowKeyPressed_(QKeyEvent *);

    void onResolutionOutput_();
    void onResolutionCustom_();
    void onResolutionPredefined_(QAction*);

    // -- network --

    void sendSceneToClients_();
private:

    void createObjects_();
    void setScene_(Scene *, const SceneSettings *set = 0);

    void updateSequenceView_(Object *o);
    void updateWindowTitle_();
    QString getSceneSaveFilename_();
    bool saveScene_(const QString& fn);
    void loadScene_(const QString& fn);

    void copySceneSettings_(Object * o);
    void setPredefinedResolution_(int index);
    void updateResolutionActions_();

    QMainWindow * window_;
    Scene * scene_;
#ifndef MO_DISABLE_TREE
    ObjectTreeModel * objectTreeModel_;
#endif
    QSize outputSize_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    ObjectView * objectView_;
#ifndef MO_DISABLE_TREE
    ObjectTreeView * objectTreeView_;
#endif
    ObjectGraphView * objectGraphView_;

    SceneSettings * sceneSettings_;

    Sequencer * sequencer_;
    ClipView * clipView_;
    SequenceView * seqView_;

    TransportWidget * transportWidget_;

    QObjectInspector * qobjectInspector_;

    ServerDialog * serverDialog_;

    QStatusBar * statusBar_;
    QLabel * sysInfoLabel_;
    QTimer * sysInfoTimer_;

    QString currentSceneFilename_;

    TestThread * testThread_;

    QMenu * menuEdit_, * menuResolutions_,
          * menuProjectorIndex_;

    QAction * actionSaveScene_,
            * aDrawLightSources_,
            * aDrawAudioSources_,
            * aDrawCameras_,
            * aDrawMicrophones_,
            * aResolutionOutput_,
            * aResolutionCustom_,
            * aResolutionPredefined_;

    bool sceneNotSaved_;

    // ----- config -------

    int statusMessageTimeout_;

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
