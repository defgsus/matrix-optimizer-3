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
namespace GL { class Window; class Context; class Manager; }
namespace GUI {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private:

    void createWidgets_();
    void createMainMenu_();
    void createObjects_();

    ObjectTreeModel * objModel_;

    GL::Manager * glManager_;
    GL::Window * glWindow_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
