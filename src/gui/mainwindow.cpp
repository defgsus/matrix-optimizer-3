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

#include "mainwindow.h"
#include "util/mainwidgetcontroller.h"
#include "io/settings.h"
#include "gl/window.h"
#include "gui/clipview.h"
#include "gui/objectview.h"
#include "gui/sequencer.h"
#include "gui/sequenceview.h"
#include "gui/objectgraphview.h"
#include "gui/widget/transportwidget.h"
#include "gui/widget/spacer.h"
#include "object/scene.h"


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

    createWidgets_();

    // read previous geometry
    restoreAllGeometry_();

    connect(controller_, SIGNAL(modeChanged()),
            this, SLOT(adjustWidgets_()));

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
            l0->addLayout(lv);
            //lv->setSizeConstraint(QLayout::SetMinAndMaxSize);

                // sequencer
                lv->addWidget(controller_->sequencer());

                // clipview
                lv->addWidget(controller_->clipView());

                // object graph
                lv->addWidget(controller_->objectGraphView());
                //controller_->objectGraphView()->setSizePolicy(
                //            QSizePolicy::Expanding, QSizePolicy::Minimum);
                //controller_->objectGraphView()->viewport()->setSizePolicy(
                //            QSizePolicy::Expanding, QSizePolicy::Minimum);

                //spacer2_ = new Spacer(Qt::Horizontal, this);
                //lv->addWidget(spacer2_);

                // Sequence view
                lv->addWidget(controller_->sequenceView());

            lv = new QVBoxLayout();
            l0->addLayout(lv);

                spacer_ = new Spacer(Qt::Vertical, this);
                lv->addWidget(spacer_);

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
                controller_->objectView()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);


        spacer_->setWidgets(rightContainer, controller_->sequencer());
        //spacer2_->setWidgets(sequencer_, seqFloatView_);



    setMinimumSize(800,500);

}

void MainWindow::adjustWidgets_()

{
    float fac = 1.0;
    if (controller_->sequenceView()->isVisible())
        fac *= 0.6;
    /*if (controller_->sequencer()->isVisible() ||
        controller_->clipView()->isVisible())
        fac *= 0.6;*/
    controller_->objectGraphView()->setMaximumHeight(height() * fac);
}


void MainWindow::saveAllGeometry_()
{
    settings->storeGeometry(this);
    if (controller_->glWindow())
        settings->storeGeometry(controller_->glWindow());
}

bool MainWindow::restoreAllGeometry_()
{
    bool r = settings->restoreGeometry(this);
    if (controller_->glWindow())
        settings->restoreGeometry(controller_->glWindow());
    return r;
}


void MainWindow::resizeEvent(QResizeEvent *)
{
    adjustWidgets_();
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




} // namespace GUI
} // namespace MO
