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

class TestThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void setEditActions_(const QObject * sender, QList<QAction*> actions);
    void testSceneTransform_();
    void updateSystemInfo_();

    void objectSelected(MO::Object*);
    void treeChanged();

    void start();
    void stop();

    void saveScene();
    void saveSceneAs();
    void loadScene();
    void newScene();

    void createDebugScene_();
    void resetTreeModel_();
    void runTestThread_();

    void setSceneObject(Scene *);

    void updateWidgetsActivity_();
private:

    void createWidgets_();
    void createMainMenu_();
    void createObjects_();

    void updateWindowTitle_();
    QString getSceneSaveFilename_();
    void saveScene_(const QString& fn);
    void loadScene_(const QString& fn);

    Scene * scene_;
    ObjectTreeModel * objectTreeModel_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    QMenu * editMenu_;
    ObjectView * objectView_;
    ObjectTreeView * objectTreeView_;

    Sequencer * sequencer_;
    SequenceFloatView * seqFloatView_;

    Spacer * spacer_, * spacer2_;

    QObjectInspector * qobjectView_;

    QTimer * sysInfoTimer_;
    QLabel * sysInfoLabel_;

    TestThread * testThread_;

    QAction * actionSaveScene_;

    QString
        sceneFilename_;

    // ---------- config -----------

    QString currentSceneDirectory_;

    int statusMessageTimeout_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
