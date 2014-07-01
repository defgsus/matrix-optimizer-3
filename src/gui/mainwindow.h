/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_GUI_MAINWINDOW_H
#define MOSRC_GUI_MAINWINDOW_H

#include <QMainWindow>

namespace MO {
class ObjectTreeModel;
class Scene;
class Object;
namespace GL { class Window; class Context; class Manager; }
namespace GUI {

class QObjectInspector;
class ObjectView;
class SequenceFloatView;

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

    void objectSelected(MO::Object*);

    void start();
    void stop();
private:

    void createWidgets_();
    void createMainMenu_();
    void createObjects_();

    ObjectTreeModel * objModel_;
    Scene * scene_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;

    QMenu * editMenu_;
    ObjectView * objectView_;

    SequenceFloatView * seqFloatView_;

    QObjectInspector * qobjectView_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
