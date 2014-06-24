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

    auto ruler = new Ruler(this);
    ruler->setFixedHeight(40);
    l->addWidget(ruler);

    auto tl = new Timeline1D;
    for (int i=0; i<200; ++i)
        tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, Timeline1D::Point::SYMMETRIC);
    tl->setAutoDerivative();

    auto tlv = new Timeline1DView(tl, this);
    l->addWidget(tlv);
}

MainWindow::~MainWindow()
{
    delete ui;
}

} // namespace GUI
} // namespace MO
