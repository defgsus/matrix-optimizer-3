/** @file

    @brief mainwindow

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/
#include <QLayout>


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectorsetupwidget.h"
#include "timeline1dview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "gui/painter/grid.h"

namespace MO {
namespace GUI {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto l = new QVBoxLayout(centralWidget());

    //auto v = new ProjectorSetupWidget(centralWidget());
    //l->addWidget(v);

    //MO_IO_ERROR("parent = " );

    auto grid = new QGridLayout();
    l->addLayout(grid);

#define HAVE_RULER

#ifdef HAVE_RULER
        auto rulerX = new Ruler(this);
        rulerX->setFixedHeight(40);
        grid->addWidget(rulerX, 0, 1);
        rulerX->setOptions(Ruler::O_DragX | Ruler::O_DrawX | Ruler::O_DrawTextX | Ruler::O_ZoomX);

        auto rulerY = new Ruler(this);
        rulerY->setFixedWidth(60);
        grid->addWidget(rulerY, 1, 0);
        rulerY->setOptions(Ruler::O_DragY | Ruler::O_DrawY | Ruler::O_DrawTextY | Ruler::O_ZoomY);
#endif

        auto tl = new Timeline1D;
        for (int i=0; i<200; ++i)
            tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, Timeline1D::Point::SYMMETRIC);
        tl->setAutoDerivative();

        auto tlv = new Timeline1DView(tl, this);
        grid->addWidget(tlv, 1, 1);
        tlv->setGridOptions(PAINTER::Grid::O_DrawX | PAINTER::Grid::O_DrawY);
#if (0)
        tlv->setOptions(
                        Timeline1DView::O_MoveViewY
                    |   Timeline1DView::O_MovePoints
                    );
#endif

#ifdef HAVE_RULER
        connect(rulerX, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), tlv, SLOT(setViewSpace(UTIL::ViewSpace)));
        connect(tlv, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX, SLOT(setViewSpace(UTIL::ViewSpace)));

        connect(rulerY, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), tlv, SLOT(setViewSpace(UTIL::ViewSpace)));
        connect(tlv, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY, SLOT(setViewSpace(UTIL::ViewSpace)));

        connect(rulerX, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerY, SLOT(setViewSpace(UTIL::ViewSpace)));
        connect(rulerY, SIGNAL(viewSpaceChanged(UTIL::ViewSpace)), rulerX, SLOT(setViewSpace(UTIL::ViewSpace)));

        rulerX->setViewSpace(rulerX->viewSpace(), true);
#endif

    // ---------- io -----------

    QAction * a;

    a = new QAction(tr("save timeline"), this);
    menuBar()->addAction(a);
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, &QAction::triggered, [=]()
    {
        tl->saveFile("./timeline.bin");
    });

    a = new QAction(tr("load timeline"), this);
    menuBar()->addAction(a);
    a->setShortcut(Qt::CTRL + Qt::Key_L);
    connect(a, &QAction::triggered, [=]()
    {
        tl->loadFile("./timeline.bin");
        tlv->unselect();
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

} // namespace GUI
} // namespace MO
