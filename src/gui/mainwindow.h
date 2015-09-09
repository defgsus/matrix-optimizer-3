/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#ifndef MOSRC_GUI_MAINWINDOW_H
#define MOSRC_GUI_MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

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

    void keyPressEvent(QKeyEvent *);

public slots:


    /** Creates and returns a default dockwidget for @p widget.
        @p widget must have an object name! */
    QDockWidget * createDockWidget(const QString& name, QWidget * widget);

private slots:

    void saveAllGeometry_();
    bool restoreAllGeometry_();

protected:

    //bool event(QEvent *event) Q_DECL_OVERRIDE; /* debug */
    void showEvent(QShowEvent *) Q_DECL_OVERRIDE;

private:

    void createWidgets_();
    void createDockWidgets_();
    void createMenus_();

    MainWidgetController * controller_;

    QMenu * viewMenu_;

    QMap<QWidget*, QDockWidget*> dockMap_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_MAINWINDOW_H
