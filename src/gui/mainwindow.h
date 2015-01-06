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

class QMenu;

namespace MO {
namespace GUI {

class MainWidgetController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);
    //void resizeEvent(QResizeEvent *);

public slots:

private slots:

    void saveAllGeometry_();
    bool restoreAllGeometry_();

private:

    void createWidgets_();
    void createDockWidgets_();
    void createMenus_();

    /** Creates and returns a default dockwidget for @p widget.
        @p widget must have an object name! */
    QDockWidget * createDockWidget_(const QString& name, QWidget * widget);

    MainWidgetController * controller_;

    QMenu * viewMenu_;

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
