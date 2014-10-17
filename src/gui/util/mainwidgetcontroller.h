/** @file mainwidgetcontroller.h

    @brief Controller for all main edit widgets

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 17.10.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
#define MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H

#include <QObject>

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
class SequenceFloatView;
class Sequencer;
class SceneSettings;
class TransportWidget;
class ServerDialog;
class ClipView;
class TestThread;

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
    ObjectTreeModel * objectTreeModel() const { return objectTreeModel_; }
    SceneSettings * sceneSettings() const { return sceneSettings_; }
    GL::Manager * glManager() const { return glManager_; }
    GL::Window * glWindow() const { return glWindow_; }
    ObjectView * objectView() const { return objectView_; }
    ObjectTreeView * objectTreeView() const { return objectTreeView_; }
    Sequencer * sequencer() const { return sequencer_; }
    ClipView * clipView() const { return clipView_; }
    SequenceFloatView * sequenceFloatView() const { return seqFloatView_; }
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

    void resetTreeModel_();
    void runTestThread_();
    void dumpIdNames_();
    void dumpNeededFiles_();
    void exportPovray_();

    void showClipView_(bool enable, Object * = 0);
    void showSequence_(bool enable, Sequence * = 0);
    void showSequencer_(bool enable, Object * = 0);

    void updateWidgetsActivity_();

    void updateNumberOutputEnvelopes_(uint);
    void updateOutputEnvelope_(const F32*);

    void onWindowKeyPressed_(QKeyEvent *);

private:

    void createObjects_();
    void setScene_(Scene *, const SceneSettings *set = 0);

    void updateSequenceView_(Object *o);
    void updateWindowTitle_();
    QString getSceneSaveFilename_();
    bool saveScene_(const QString& fn);
    void loadScene_(const QString& fn);


    QMainWindow * window_;
    Scene * scene_;
    ObjectTreeModel * objectTreeModel_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    ObjectView * objectView_;
    ObjectTreeView * objectTreeView_;

    SceneSettings * sceneSettings_;

    Sequencer * sequencer_;
    ClipView * clipView_;
    SequenceFloatView * seqFloatView_;

    TransportWidget * transportWidget_;

    QObjectInspector * qobjectInspector_;

    ServerDialog * serverDialog_;

    QStatusBar * statusBar_;
    QLabel * sysInfoLabel_;
    QTimer * sysInfoTimer_;

    QString currentSceneFilename_;

    TestThread * testThread_;

    QMenu * editMenu_;

    QAction * actionSaveScene_,
            * aDrawLightSources_,
            * aDrawAudioSources_,
            * aDrawCameras_,
            * aDrawMicrophones_;

    bool sceneNotSaved_;

    // ----- config -------

    int statusMessageTimeout_;

};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_MAINWIDGETCONTROLLER_H
