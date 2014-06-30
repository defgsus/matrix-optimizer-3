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
#include <QTreeView>
#include <QCloseEvent>

#include "mainwindow.h"
#include "projectorsetupwidget.h"
#include "timeline1dview.h"
#include "timeline1drulerview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "gui/painter/grid.h"
#include "gui/qobjectinspector.h"
#include "gui/objecttreeview.h"
#include "model/objecttreemodel.h"
#include "io/datastream.h"
#include "gl/manager.h"
#include "gl/window.h"

#include "object/objectfactory.h"
#include "object/object.h"
#include "object/scene.h"

namespace MO {
namespace GUI {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(tr("Matrix Optimizer 3.0"));

    setAttribute(Qt::WA_DeleteOnClose, true);

    createMainMenu_();
    createWidgets_();
    createObjects_();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createWidgets_()
{
    setMinimumSize(800,400);

    setCentralWidget(new QWidget(this));

    auto l0 = new QHBoxLayout(centralWidget());

        auto treev = new ObjectTreeView(this);
        l0->addWidget(treev);

        objModel_ = new ObjectTreeModel(0, this);
        treev->setModel(objModel_);
        connect(treev, SIGNAL(editActionsChanged(const QObject*,QList<QAction*>)),
                SLOT(setEditActions_(const QObject*,QList<QAction*>)));

        auto l = new QVBoxLayout();
        l0->addLayout(l);

            //auto v = new ProjectorSetupWidget(centralWidget());
            //l->addWidget(v);

            auto tl = new MATH::Timeline1D;
            for (int i=0; i<200; ++i)
                tl->add((Double)rand()/RAND_MAX * 10.0, (Double)rand()/RAND_MAX, MATH::Timeline1D::Point::SYMMETRIC);
            tl->setAutoDerivative();

            auto tlv = new Timeline1DRulerView(tl, this);
            tlv->setObjectName("timeline01");
            l->addWidget(tlv);
            //tlv->setOptions(Timeline1DView::O_ChangeViewX | Timeline1DView::O_MovePoints);
            //tlv->setGridOptions(PAINTER::Grid::O_DrawX | PAINTER::Grid::O_DrawY);
            auto space = tlv->viewSpace();
            space.setMinX(0);
            space.setMinY(0);
            space.setMaxX(10);
            space.setMaxY(1);
            tlv->setViewSpace(space, true);

            auto tl2 = new MATH::Timeline1D;
            tl2->setLimit(0,1);

            auto tlv2 = new Timeline1DRulerView(tl2, this);
            tlv2->setObjectName("timeline02");
            l->addWidget(tlv2);

    // --------- io ----------

    QMenu * m;
    QAction * a;

    m = new QMenu(tr("Timeline"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("save timeline"), m);
        m->addAction(a);
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, &QAction::triggered, [=]()
        {
            tl->saveFile("./timeline.bin");
        });

        a = new QAction(tr("load timeline"), m);
        m->addAction(a);
        a->setShortcut(Qt::CTRL + Qt::Key_L);
        connect(a, &QAction::triggered, [=]()
        {
            tl->loadFile("./timeline.bin");
            tlv->unselect();
        });

}


void MainWindow::createMainMenu_()
{
    QMenu * m;
    QAction * a;

    m = editMenu_ = new QMenu(tr("Edit"), menuBar());
    menuBar()->addMenu(m);

    m = new QMenu(tr("Debug"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("QObject inspector"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            (new QObjectInspector(this, this))->show();
        });

}


void MainWindow::createObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow();
    glWindow_->show();

    auto scene = ObjectFactory::createSceneObject();
    scene->setParent(this);

    connect(glManager_, SIGNAL(renderRequest()), scene, SLOT(renderScene()));
    connect(glManager_, SIGNAL(contextCreated(MO::GL::Context*)), scene, SLOT(setGlContext(MO::GL::Context*)));

    auto cam = scene->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_CAMERA));
        //cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
        //cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
        auto mic = cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_MICROPHONE));
            mic->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
    auto model = scene->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_MODEL3D));
        model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
        model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
        auto snd = model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_SOUNDSOURCE));
            snd->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));
            snd->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSFORMATION));

    //mic->setParentObject(snd);
    //snd->addObject(mic);
/*
    QByteArray bytes;
    {
        IO::DataStream io(&bytes, QIODevice::WriteOnly);
        cam->serializeTree(io);
    }
    {
        IO::DataStream io(&bytes, QIODevice::ReadOnly);
        auto obj = Object::deserializeTree(io);
        scene->addObject(obj);
    }
*/
    objModel_->setRootObject(scene);
}



void MainWindow::closeEvent(QCloseEvent * e)
{
    QMainWindow::closeEvent(e);
/*
    if (e->isAccepted())
    {

    }
*/
}


void MainWindow::setEditActions_(const QObject *, QList<QAction *> actions)
{
    editMenu_->clear();
    editMenu_->addActions(actions);
}

} // namespace GUI
} // namespace MO
