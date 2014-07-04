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
#include <QTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>

#include "mainwindow.h"
#include "projectorsetupwidget.h"
#include "timeline1dview.h"
#include "timeline1drulerview.h"
#include "ruler.h"
#include "math/timeline1d.h"
#include "gui/painter/grid.h"
#include "gui/qobjectinspector.h"
#include "gui/objecttreeview.h"
#include "gui/objectview.h"
#include "gui/sequencefloatview.h"
#include "gui/sequencer.h"
#include "model/objecttreemodel.h"
#include "io/datastream.h"
#include "gl/manager.h"
#include "gl/window.h"
#include "io/error.h"


#include "object/objectfactory.h"
#include "object/object.h"
#include "object/scene.h"
#include "object/sequencefloat.h"

namespace MO {
namespace GUI {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow     (parent),
    scene_          (0),
    objectModel_    (0),
    seqFloatView_   (0),
    qobjectView_    (0)
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
    setCentralWidget(new QWidget(this));
    centralWidget()->setObjectName("_centralwidget");

    auto l0 = new QHBoxLayout(centralWidget());

        auto lv = new QVBoxLayout();
        l0->addLayout(lv);

            // object tree view
            objectTreeView_ = new ObjectTreeView(this);
            lv->addWidget(objectTreeView_);
            objectTreeView_->setMinimumWidth(200);
            objectTreeView_->setMaximumWidth(450);

            objectModel_ = new ObjectTreeModel(0, this);
            objectTreeView_->setModel(objectModel_);
            connect(objectTreeView_, SIGNAL(editActionsChanged(const QObject*,QList<QAction*>)),
                    SLOT(setEditActions_(const QObject*,QList<QAction*>)));

            // object editor
            objectView_ = new ObjectView(this);
            lv->addWidget(objectView_);

            connect(objectTreeView_, SIGNAL(objectSelected(MO::Object*)),
                    SLOT(objectSelected(MO::Object*)));

        l0->setStretchFactor(lv, -1);

        lv = new QVBoxLayout();
        l0->addLayout(lv);

            //auto sa = new QScrollArea(this);
            //lv->addWidget(sa);
            sequencer_ = new Sequencer(this);
            lv->addWidget(sequencer_);
            //sa->setWidget(trackView_);
            //trackView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


            // SequenceFloat view
            seqFloatView_ = new SequenceFloatView(this);
            seqFloatView_->setVisible(false);
            lv->addWidget(seqFloatView_);


    // --------- io ----------
/*
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
*/
    setMinimumSize(800,400);

}


void MainWindow::createMainMenu_()
{
    QMenu * m;
    QAction * a;

    // ######### FILE MENU #########
    m = editMenu_ = new QMenu(tr("File"), menuBar());
    menuBar()->addMenu(m);

    m->addAction(a = new QAction(tr("Load scene"), menuBar()));
    connect(a, SIGNAL(triggered()), this, SLOT(loadScene()));

    m->addAction(a = new QAction(tr("Save scene as"), menuBar()));
    connect(a, SIGNAL(triggered()), this, SLOT(saveScene()));

    m->addAction(a = new QAction(tr("New scene"), menuBar()));
    connect(a, SIGNAL(triggered()), this, SLOT(newScene()));

    // ######### EDIT MENU #########
    m = editMenu_ = new QMenu(tr("Edit"), menuBar());
    menuBar()->addMenu(m);
    // will be updated from child widgets

    // ######### RENDER MENU #########
    m = new QMenu(tr("Render"), menuBar());
    menuBar()->addMenu(m);

    QActionGroup * ag = new QActionGroup(menuBar());
    m->addAction(a = new QAction(tr("Start"), menuBar()));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F7);
    a->setCheckable(true);
    connect(a, SIGNAL(triggered()), this, SLOT(start()));

    m->addAction(a = new QAction(tr("Stop"), menuBar()));
    ag->addAction(a);
    a->setShortcut(Qt::Key_F8);
    a->setCheckable(true);
    a->setChecked(true);
    connect(a, SIGNAL(triggered()), this, SLOT(stop()));

    // ######### DEBUG MENU #########
    m = new QMenu(tr("Debug"), menuBar());
    menuBar()->addMenu(m);

        a = new QAction(tr("QObject inspector"), m);
        m->addAction(a);
        connect(a, &QAction::triggered, [=]()
        {
            if (!qobjectView_)
                qobjectView_ = new QObjectInspector(this, this);
            qobjectView_->setRootObject(this);
            qobjectView_->show();
        });


        m->addAction(a = new QAction(tr("Test transformation speed"), m));
        connect(a, SIGNAL(triggered()), SLOT(testSceneTransform_()));
}

void MainWindow::setSceneObject(Scene * s)
{
    MO_ASSERT(s, "MainWindow::setSceneObject() with NULL scene");

    if (scene_)
        scene_->deleteLater();

    scene_ = s;

    // manage memory
    scene_->setParent(this);

    MO_ASSERT(glManager_ && glWindow_, "");

    // connect to render window
    connect(glManager_, SIGNAL(renderRequest()), scene_, SLOT(renderScene()));
    connect(glManager_, SIGNAL(contextCreated(MO::GL::Context*)),
                scene_, SLOT(setGlContext(MO::GL::Context*)));

    connect(scene_, SIGNAL(renderRequest()), glWindow_, SLOT(renderLater()));

    if (glWindow_->context())
        scene_->setGlContext(glWindow_->context());

    // update widgets

    objectModel_->setRootObject(scene_);
    objectView_->setObject(0);
    seqFloatView_->setSequence(0);

    glWindow_->renderLater();
}


void MainWindow::createObjects_()
{
    glManager_ = new GL::Manager(this);
    glWindow_ = glManager_->createGlWindow();
    glWindow_->show();

    newScene();
    /*
    auto scene = ObjectFactory::createSceneObject();

    auto cam = scene->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_CAMERA));
        cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSLATION));
        cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_AXISROTATION));
        cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_SEQUENCE_FLOAT));
        cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_SEQUENCE_FLOAT));
        auto mic = cam->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_MICROPHONE));
            mic->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_AXISROTATION));
    auto model = scene->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_MODEL3D));
        model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_AXISROTATION));
        model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSLATION));
        model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_AXISROTATION));
        auto snd = model->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_SOUNDSOURCE));
            snd->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_AXISROTATION));
            snd->addObject(ObjectFactory::createObject(MO_OBJECTCLASSNAME_TRANSLATION));

    setSceneObject(scene);
    */
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

void MainWindow::objectSelected(Object * o)
{
    objectView_->setObject(o);

    if (o && o->type() == Object::T_SEQUENCE_FLOAT)
    {
        seqFloatView_->setSequence(static_cast<SequenceFloat*>(o));
        if (!seqFloatView_->isVisible())
            seqFloatView_->setVisible(true);
    }
    else
        seqFloatView_->setVisible(false);

}

void MainWindow::setEditActions_(const QObject *, QList<QAction *> actions)
{
    editMenu_->clear();
    editMenu_->addActions(actions);
}

void MainWindow::testSceneTransform_()
{
    QTime t;

    int num = 10000000;
    int i = 0;
    int e = 0;

    t.start();
    for (; i < num && e <= 1000; ++i)
    {
        for (int j=0; j<1000; ++j, ++i)
            scene_->calculateSceneTransform(0, 0.01 * i);

        e = t.elapsed();
    }
    num = i;
    const Double elapsed = (Double)e / 1000.0;

    QMessageBox::information(this, tr("Scene transformation test"),
        tr("Calculating %1 frames of transformation\n"
           "which took %2 seconds.\n"
           "This is %3 milli-seconds per frame\n"
           "and %4 frames per second.")
                             .arg(num)
                             .arg(elapsed)
                             .arg((elapsed*1000)/num)
                             .arg((int)((Double)num/elapsed))
           );
}

void MainWindow::start()
{
    scene_->start();
}

void MainWindow::stop()
{
    scene_->stop();
}

void MainWindow::newScene()
{
    setSceneObject( ObjectFactory::createSceneObject() );
}

void MainWindow::saveScene()
{
    if (!scene_)
        return;

    QString fn = QFileDialog::getSaveFileName(this, tr("Save Scene"),
                                              "./", "MatrixOptimizer (*.mo3)");
    if (fn.isEmpty())
        return;

    ObjectFactory::saveScene(fn, scene_);
}

void MainWindow::loadScene()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Load Scene"),
                                              "./", "MatrixOptimizer (*.mo3)");
    if (fn.isEmpty())
        return;

    Scene * scene = ObjectFactory::loadScene(fn);

    if (scene)
        setSceneObject(scene);
}

} // namespace GUI
} // namespace MO
