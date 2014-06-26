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
#include "timeline1drulerview.h"
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

        auto tl = new MATH::Timeline1D;
        for (int i=0; i<200; ++i)
            tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, MATH::Timeline1D::Point::SYMMETRIC);
        tl->setAutoDerivative();

        auto tlv = new Timeline1DRulerView(tl, this);
        l->addWidget(tlv);
        tlv->setOptions(Timeline1DView::O_MoveViewX);
        //tlv->setGridOptions(PAINTER::Grid::O_DrawX | PAINTER::Grid::O_DrawY);

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
