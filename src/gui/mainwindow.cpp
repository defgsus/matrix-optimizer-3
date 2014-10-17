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
#include "gui/objecttreeview.h"
#include "gui/objectview.h"
#include "gui/sequencer.h"
#include "gui/sequencefloatview.h"
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

        // main layout
        auto l0 = new QHBoxLayout();
        lv0->addLayout(l0);
        l0->setMargin(0);
        l0->setSpacing(1);

            auto leftContainer = new QWidget(this);
            l0->addWidget(leftContainer);
            leftContainer->setObjectName("_left_container");
            leftContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            leftContainer->setMinimumWidth(340);

            auto lv = new QVBoxLayout(leftContainer);
            lv->setMargin(0);
            //lv->setSizeConstraint(QLayout::SetMinAndMaxSize);

                // object tree view
                lv->addWidget(controller_->objectTreeView());
                controller_->objectTreeView()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
                //objectTreeView_->setMinimumWidth(320);
                //objectTreeView_->setMaximumWidth(450);

                // object editor
                lv->addWidget(controller_->objectView());
                controller_->objectView()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                //objectView_->setMinimumWidth(320);
            //l0->setStretchFactor(lv, -1);

            lv = new QVBoxLayout();
            l0->addLayout(lv);

                spacer_ = new Spacer(Qt::Vertical, this);
                lv->addWidget(spacer_);

            lv = new QVBoxLayout();
            l0->addLayout(lv);

                // sequencer
                lv->addWidget(controller_->sequencer());

                // clipview
                lv->addWidget(controller_->clipView());

                //spacer2_ = new Spacer(Qt::Horizontal, this);
                //lv->addWidget(spacer2_);

                // SequenceFloat view
                lv->addWidget(controller_->sequenceFloatView());


        spacer_->setWidgets(leftContainer, controller_->sequencer());
        //spacer2_->setWidgets(sequencer_, seqFloatView_);



    setMinimumSize(800,500);

}




void MainWindow::saveAllGeometry_()
{
    settings->saveGeometry(this);
    if (controller_->glWindow())
        settings->saveGeometry(controller_->glWindow());
}

bool MainWindow::restoreAllGeometry_()
{
    bool r = settings->restoreGeometry(this);
    if (controller_->glWindow())
        settings->restoreGeometry(controller_->glWindow());
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




} // namespace GUI
} // namespace MO
