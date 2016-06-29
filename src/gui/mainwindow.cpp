/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2014/04/21</p>
*/

#include <QDebug>
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QCloseEvent>
#include <QDockWidget>

#include "mainwindow.h"
#include "util/mainwidgetcontroller.h"
#include "io/settings.h"
#include "gl/window.h"
#include "gui/clipview.h"
#include "gui/objectview.h"
#include "gui/serverview.h"
#include "gui/sequencer.h"
#include "gui/sequenceview.h"
#include "gui/objectgraphview.h"
#include "gui/frontview.h"
#include "gui/frontitemeditor.h"
#include "gui/widget/objectoutputview.h"
#include "gui/widget/transportwidget.h"
#include "gui/widget/assetbrowser.h"
#include "gui/widget/spacer.h"
#include "gui/widget/objecttreeview.h"
#include "gui/oscview.h"
#include "object/scene.h"
#include "io/application.h"
#include "io/keyboardstate.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {





MainWindow::MainWindow(QWidget *parent) :
    QMainWindow     (parent),
    controller_     (0)
{
    setObjectName("_MainWindow");

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(QIcon(":/icon/mo.png"));

    // create controller with all main widgets
    controller_ = new MainWidgetController(this);

    createMenus_();

    //createWidgets_();
    createDockWidgets_();


    // read previous geometry
    restoreAllGeometry_();

    controller_->initScene();    
}

MainWindow::~MainWindow()
{

}

void MainWindow::createWidgets_()
{
    // main container
    setCentralWidget(new QWidget(this));
    centralWidget()->setObjectName("_centralwidget");

    // menu and statusbar
    controller_->createMainMenu(menuBar());
    setStatusBar(controller_->statusBar());

    // window title
    connect(controller_, SIGNAL(windowTitle(QString)),
            this, SLOT(setWindowTitle(QString)));

    setMinimumSize(800,500);

}

void MainWindow::createDockWidgets_()
{
    auto dock = createDockWidget(tr("Transport"), controller_->transportWidget());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);

    dock = createDockWidget(tr("Scene"), controller_->objectGraphView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);

    dock = createDockWidget(tr("Tree"), controller_->objectTreeView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);

#ifndef MO_DISABLE_FRONT
    dock = createDockWidget(tr("Frontend"), controller_->frontView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);
    removeDockWidget(dock);
#endif

    dock = createDockWidget(tr("Sequence"), controller_->sequenceView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);

    dock = createDockWidget(tr("Tracks"), controller_->sequencer());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);
    removeDockWidget(dock);

    dock = createDockWidget(tr("Clips"), controller_->clipView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);
    removeDockWidget(dock);

    dock = createDockWidget(tr("Object"), controller_->objectView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Horizontal);

    dock = createDockWidget(tr("Browser"), controller_->assetBrowser());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Horizontal);

    dock = createDockWidget(tr("OSC Listeners"), controller_->oscView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Horizontal);

    dock = createDockWidget(tr("Object outputs"), controller_->objectOutputView());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);

#ifndef MO_DISABLE_FRONT
    dock = createDockWidget(tr("Frontend settings"), controller_->frontItemEditor());
    addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Horizontal);
    removeDockWidget(dock);
#endif

#ifndef MO_DISABLE_SERVER
    if (isServer())
    {
        dock = createDockWidget(tr("Server/Client"), controller_->serverView());
        addDockWidget(Qt::AllDockWidgetAreas, dock, Qt::Vertical);
        removeDockWidget(dock);
    }
#endif
}

QDockWidget * MainWindow::createDockWidget(const QString &name, QWidget *widget)
{
    MO_ASSERT(!widget->objectName().isEmpty(), "need name for layout reload");

    if (dockMap_.contains(widget))
        return dockMap_.value(widget);

    auto dock = new QDockWidget(name, this);
    dock->setObjectName(widget->objectName() + "_Dock");
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setWidget(widget);
    dock->setFeatures(  QDockWidget::DockWidgetMovable
                      | QDockWidget::DockWidgetFloatable
                      | QDockWidget::DockWidgetClosable
                      );
    auto act = dock->toggleViewAction();
    viewMenu_->addAction( act );

    dockMap_.insert(widget, dock);

    return dock;
}


void MainWindow::createMenus_()
{
    // menu and statusbar
    controller_->createMainMenu(menuBar());
    setStatusBar(controller_->statusBar());

    // window title
    connect(controller_, SIGNAL(windowTitle(QString)),
            this, SLOT(setWindowTitle(QString)));

    viewMenu_ = controller_->viewMenu();
}

/*
bool MainWindow::event(QEvent* e)
{
    MO_PRINT("MainWindow::event(" << e << ")");
    return QMainWindow::event(e);
}*/


void MainWindow::showEvent(QShowEvent * e)
{
    QMainWindow::showEvent(e);
    application()->checkVersionUpdate();
}


void MainWindow::saveAllGeometry_()
{
    settings()->storeGeometry(this);
    if (controller_->glWindow())
        settings()->storeGeometry(controller_->glWindow());
}

bool MainWindow::restoreAllGeometry_()
{
    bool r = settings()->restoreGeometry(this);
    if (controller_->glWindow())
        settings()->restoreGeometry(controller_->glWindow());
    return r;
}


void MainWindow::closeEvent(QCloseEvent * e)
{
    if (controller_->isOkayToChangeScene())
    {
        e->accept();

        saveAllGeometry_();

        //if (controller_->scene())
        //    controller_->scene()->kill();
    }
    else
        e->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
    KeyboardState::globalInstance().keyDown(e->key());
}

void MainWindow::keyReleaseEvent(QKeyEvent * e)
{
    KeyboardState::globalInstance().keyUp(e->key());
}


} // namespace GUI
} // namespace MO
