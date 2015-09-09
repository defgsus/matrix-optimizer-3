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
#include "object/scene.h"
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

    // main-main layout
    auto lv0 = new QVBoxLayout(centralWidget());
    lv0->setMargin(0);

        lv0->addWidget(controller_->transportWidget());

        // ------ main layout ------
        auto l0 = new QHBoxLayout();
        lv0->addLayout(l0);
        l0->setMargin(0);
        l0->setSpacing(1);

            // --- left container ----
            auto lv = new QVBoxLayout();
            l0->addLayout(lv, 1);
            //lv->setSizeConstraint(QLayout::SetMinAndMaxSize);

                // sequencer
                lv->addWidget(controller_->sequencer());

                // clipview
                lv->addWidget(controller_->clipView());

                // object graph
                lv->addWidget(controller_->objectGraphView());
                controller_->objectGraphView()->setMinimumWidth(320);
                controller_->objectGraphView()->setSizePolicy(
                            QSizePolicy::Expanding, QSizePolicy::Expanding);

                // Sequence view
                lv->addWidget(controller_->sequenceView());

            lv = new QVBoxLayout();
            l0->addLayout(lv);

            // ------ right container -----
            auto rightContainer = new QWidget(this);
            l0->addWidget(rightContainer);
            rightContainer->setObjectName("_right_container");
            rightContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            rightContainer->setMinimumWidth(320);

            auto ll = new QHBoxLayout(rightContainer);
            ll->setMargin(0);
            //lv->setSizeConstraint(QLayout::SetMinAndMaxSize);


                // object (parameter) editor
                ll->addWidget(controller_->objectView());
                controller_->objectView()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);


    setMinimumSize(800,500);

}

void MainWindow::createDockWidgets_()
{
    auto dock = createDockWidget(tr("Transport"), controller_->transportWidget());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);

    dock = createDockWidget(tr("Scene"), controller_->objectGraphView());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);

    dock = createDockWidget(tr("Frontend"), controller_->frontView());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);
    removeDockWidget(dock);

    dock = createDockWidget(tr("Sequence"), controller_->sequenceView());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);

    dock = createDockWidget(tr("Tracks"), controller_->sequencer());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);
    removeDockWidget(dock);

    dock = createDockWidget(tr("Clips"), controller_->clipView());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);
    removeDockWidget(dock);

    dock = createDockWidget(tr("Object"), controller_->objectView());
    addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);

    dock = createDockWidget(tr("Browser"), controller_->assetBrowser());
    addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Horizontal);

    dock = createDockWidget(tr("Object outputs"), controller_->objectOutputView());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Vertical);

    dock = createDockWidget(tr("Frontend settings"), controller_->frontItemEditor());
    addDockWidget(Qt::LeftDockWidgetArea, dock, Qt::Horizontal);
    removeDockWidget(dock);

#ifndef MO_DISABLE_SERVER
    if (isServer())
    {
        dock = createDockWidget(tr("Server/Client"), controller_->serverView());
        addDockWidget(Qt::RightDockWidgetArea, dock, Qt::Vertical);
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

    viewMenu_ = new QMenu("View", menuBar());
    menuBar()->addMenu(viewMenu_);
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

    /** @todo scene description window comes to early, e.g. before actual window.. */
    //if (controller_->scene() && controller_->scene()->showSceneDesc())
    //    controller_->showSceneDesc();
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

        if (controller_->scene())
            controller_->scene()->kill();
    }
    else
        e->ignore();
}

void MainWindow::keyPressEvent(QKeyEvent * e)
{
    controller_->scene()->keyDown(e->key());
}


} // namespace GUI
} // namespace MO
