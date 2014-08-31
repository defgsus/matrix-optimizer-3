/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_GUI_MAINWINDOW_H
#define MOSRC_GUI_MAINWINDOW_H

#include <QMainWindow>

#include "object/object_fwd.h"
#include "types/float.h"

class QLabel;
class QTimer;

namespace MO {
namespace GL { class Window; class Context; class Manager; }
namespace GUI {

class QObjectInspector;
class ObjectView;
class ObjectTreeView;
class SequenceFloatView;
class Sequencer;
class Spacer;
class SceneSettings;
class TransportWidget;

class TestThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool isPlayback() const;

protected:
    void closeEvent(QCloseEvent *);

public slots:
    void start();
    void stop();

    bool saveScene();
    void saveSceneAs();
    void loadScene();
    void newScene();

    void renderToDisk();

private slots:
    void setEditActions_(const QObject * sender, QList<QAction*> actions);
    void testSceneTransform_();
    void updateSystemInfo_();
    void updateDebugRender_();

    void objectSelected_(MO::Object*);
    void treeChanged_();
    /** To trigger sceneNotSaved_ */
    void sceneChanged_();

    bool okayToChangeScene_();

    void createDebugScene_();
    void resetTreeModel_();
    void runTestThread_();
    void dumpIdNames_();
    void exportPovray_();

    void setSceneObject(Scene *, const SceneSettings * = 0);

    void updateWidgetsActivity_();

    void saveAllGeometry_();
    bool restoreAllGeometry_();

    void updateNumberOutputEnvelopes_(uint);
    void updateOutputEnvelope_(const F32*);

    void onWindowKeyPressed_(const QKeyEvent *);
private:

    void createWidgets_();
    void createMainMenu_();
    void createObjects_();

    void updateWindowTitle_();
    QString getSceneSaveFilename_();
    bool saveScene_(const QString& fn);
    void loadScene_(const QString& fn);
    /*
    void openAudio_();
    void closeAudio_();
    void startAudio_();
    void stopAudio_();
    */

    Scene * scene_;
    ObjectTreeModel * objectTreeModel_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    QMenu * editMenu_;
    ObjectView * objectView_;
    ObjectTreeView * objectTreeView_;

    SceneSettings * sceneSettings_;

    Sequencer * sequencer_;
    SequenceFloatView * seqFloatView_;

    TransportWidget * transport_;

    Spacer * spacer_, * spacer2_;

    QObjectInspector * qobjectView_;

    QTimer * sysInfoTimer_;
    QLabel * sysInfoLabel_;

    TestThread * testThread_;

    QAction * actionSaveScene_,
            * aDrawLightSources_,
            * aDrawAudioSources_,
            * aDrawCameras_,
            * aDrawMicrophones_;

    bool sceneNotSaved_;

    // ---------- config -----------

    QString
        currentSceneFilename_,
        currentSceneDirectory_;

    int statusMessageTimeout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
