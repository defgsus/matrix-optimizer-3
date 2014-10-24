/** @file projectorsetupdialog.cpp

    @brief Dialog to setup and preview projectors and mapping

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QCloseEvent>
#include <QToolButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>

#include "projectorsetupdialog.h"
#include "widget/domepreviewwidget.h"
#include "widget/spinbox.h"
#include "widget/doublespinbox.h"
#include "widget/groupwidget.h"
#include "widget/overlapareaeditwidget.h"
#include "projection/projectionsystemsettings.h"
#include "io/xmlstream.h"
#include "io/log.h"
#include "io/files.h"
#include "io/settings.h"
#include "engine/serverengine.h"
#include "projection/projectormapper.h"
#include "projection/testprojectionrenderer.h"
#include "projection/projectorblender.h"

namespace MO {
namespace GUI {


ProjectorSetupDialog::ProjectorSetupDialog(QWidget *parent)
    : QDialog       (parent),
      closeRequest_ (false),
      saidNoAlready_(false),
      settings_     (new ProjectionSystemSettings()),
      orgSettings_  (new ProjectionSystemSettings()),
      domeSettings_ (new DomeSettings()),
      projectorSettings_(new ProjectorSettings()),
      copyOfProjectorSettings_(0),
      cameraSettings_   (new CameraSettings()),
      copyOfCameraSettings_   (0),
      testRenderer_ (0)
{
    setObjectName("ProjectorSetupDialog");
    setMinimumSize(760,600);

    settings->restoreGeometry(this);

    createWidgets_();
    createMenu_();

    // init default settings
    loadDefault_();

    // XXX MH Temporary Fix for disabled menus:
    //loadPreset_();

    // fixes problems on MAC
    setWindowModality(Qt::WindowModal);

    setViewDirection(Basic3DWidget::VD_FRONT);
}

ProjectorSetupDialog::~ProjectorSetupDialog()
{
    settings->saveGeometry(this);

    delete copyOfCameraSettings_;
    delete cameraSettings_;
    delete copyOfProjectorSettings_;
    delete projectorSettings_;
    delete domeSettings_;
    delete orgSettings_;
    delete settings_;

    //delete mainMenu_;
}

void ProjectorSetupDialog::createMenu_()
{
    QMenu * menu;
    QAction * a;

    // ############### FILE ###############

    mainMenu_->addMenu(menu = new QMenu(tr("File"), mainMenu_));

        menu->addAction(a = new QAction(tr("New setup"), menu));
        a->setShortcut(Qt::CTRL + Qt::Key_N);
        connect(a, SIGNAL(triggered()), this, SLOT(clearPreset_()));

        menu->addSeparator();

        menu->addAction(a = new QAction(tr("Load setup ..."), menu));
        a->setShortcut(Qt::CTRL + Qt::Key_L);
        connect(a, SIGNAL(triggered()), this, SLOT(loadPreset_()));

        menu->addSeparator();

        menu->addAction(a = new QAction(tr("Save setup"), menu));
        a->setShortcut(Qt::CTRL + Qt::Key_S);
        connect(a, SIGNAL(triggered()), this, SLOT(savePresetAuto_()));

        menu->addAction(a = new QAction(tr("Save setup as ..."), menu));
        a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
        connect(a, SIGNAL(triggered()), this, SLOT(savePresetChoose_()));

        menu->addSeparator();

        menu->addAction(a = new QAction(tr("Load default"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(loadDefault_()));

        menu->addAction(a = new QAction(tr("Save as default"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(saveDefault_()));


    // ############### EDIT ###############

    mainMenu_->addMenu(menu = new QMenu(tr("Edit"), mainMenu_));

        menu->addAction(a = new QAction(tr("Previous projector"), menu));
        a->setShortcut(Qt::CTRL + Qt::Key_Left);
        connect(a, SIGNAL(triggered()), this, SLOT(previousProjector_()));
        aPrevious_ = a;

        menu->addAction(a = new QAction(tr("Next projector"), menu));
        a->setShortcut(Qt::CTRL + Qt::Key_Right);
        connect(a, SIGNAL(triggered()), this, SLOT(nextProjector_()));
        aNext_ = a;

        menu->addSeparator();

        menu->addAction(a = new QAction(tr("Copy projector settings"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(copyProjector_()));

        menu->addAction(a = new QAction(tr("Paste projector settings"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(pasteProjector_()));
        aPasteProjector_ = a;

        menu->addSeparator();

        menu->addAction(a = new QAction(tr("Copy camera settings"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(copyCamera_()));

        menu->addAction(a = new QAction(tr("Paste camera settings"), menu));
        connect(a, SIGNAL(triggered()), this, SLOT(pasteCamera_()));
        aPasteCamera_ = a;

#ifdef __APPLE__
        mainMenu_->setNativeMenuBar(false);
#endif

    updateActions_();
}

void ProjectorSetupDialog::createWidgets_()
{
    auto lv00 = new QVBoxLayout(this);

    mainMenu_ = new QMenuBar(this);
    lv00->addWidget(mainMenu_);

    auto lh = new QHBoxLayout();
    lv00->addLayout(lh);

        auto lv0 = new QVBoxLayout();
        lh->addLayout(lv0);


            // ------ projector select ------------

            auto lh2 = new QHBoxLayout();
            lv0->addLayout(lh2);

                comboProj_ = new QComboBox(this);
                lh2->addWidget(comboProj_);
                connect(comboProj_, SIGNAL(currentIndexChanged(int)),
                        this, SLOT(projectorSelected_()));

                auto tbut = tbAdd_ = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/new_letters.png"));
                connect(tbut, SIGNAL(clicked()), this, SLOT(newProjector_()));

                tbut = tbDup_ = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setText(tr("DUP"));
                connect(tbut, SIGNAL(clicked()), this, SLOT(duplicateProjector_()));

                tbut = tbRemove_ = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/delete.png"));
                connect(tbut, SIGNAL(clicked()), this, SLOT(deleteProjector_()));

            // --- projector setup ---

            auto frame = new QFrame(this);
            lv0->addWidget(frame);
            frame->setFrameStyle(QFrame::Raised | QFrame::Panel);

                auto lv = new QVBoxLayout(frame);
                lv->setMargin(2);

                // --------- projector settings --------------

                GroupWidget * gr;
                projectorGroup_ = gr = new GroupWidget(tr("projector settings"));
                gr->setExpanded(true);
                lv->addWidget(gr);

                editId_ = createEdit_(gr, tr("id"), tr("The internal id of the projector - don't bother"),
                                      "0", 0, true);

                editName_ = createEdit_(gr, tr("name"),
                                            tr("Some name to identify the projector"),
                                        "projector", SLOT(updateProjectorName_()));

                auto label = new QLabel(tr("view"), this);
                projectorGroup_->addWidget(label);

                spinWidth_ = createSpin_(gr, tr("width"),
                                             tr("Projector's horizontal resolution in pixels"),
                                             1024, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinWidth_->setSuffix(" " + tr("px"));

                spinHeight_ = createSpin_(gr, tr("height"),
                                             tr("Projector's vertical resolution in pixels"),
                                             768, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinHeight_->setSuffix(" " + tr("px"));

                spinFov_ = createDoubleSpin_(gr, tr("field of view"),
                                             tr("Projector's (vertical) projection angle in degree"),
                                             60, 1, 1, 180, SLOT(updateProjectorSettings_()));
                spinFov_->setSuffix(" " + tr("°"));
#ifndef MO_DISABLE_PROJECTOR_LENS_RADIUS
                spinLensRad_ = createDoubleSpin_(gr, tr("lens radius"),
                                             tr("The radius of the projector's lens in centimeters"),
                                             0, 0.1, 0, 1000, SLOT(updateProjectorSettings_()));
                spinLensRad_->setSuffix(" " + tr(" cm"));
#endif
                label = new QLabel(tr("position"), this);
                gr->addWidget(label);

                spinDist_ = createDoubleSpin_(gr, tr("distance"),
                                             tr("Projector's distance to the dome periphery "
                                                "in centimeters - "
                                                "negative is inside, positive is outside of dome"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));
                spinDist_->setSuffix(" " + tr("cm"));

                spinLat_ = createDoubleSpin_(gr, tr("latitude / azimuth"),
                                             tr("Projector's position around the dome"),
                                             0, 1, -360, 360, SLOT(updateProjectorSettings_()));
                spinLat_->setSuffix(" " + tr("°"));

                spinLong_ = createDoubleSpin_(gr, tr("longitude / elevation"),
                                             tr("Projector's height in the dome"),
                                             0, 1, -90, 90, SLOT(updateProjectorSettings_()));
                spinLong_->setSuffix(" " + tr("°"));

                label = new QLabel(tr("orientation"), this);
                gr->addWidget(label);

                spinPitch_ = createDoubleSpin_(gr, tr("pitch (x)"),
                                             tr("The x rotation of the projector's direction "
                                                "- up and down"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinPitch_->setSuffix(" " + tr("°"));

                spinYaw_ = createDoubleSpin_(gr, tr("yaw (y)"),
                                             tr("The y rotation of the projector's direction "
                                                "- left and right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinYaw_->setSuffix(" " + tr("°"));

                spinRoll_ = createDoubleSpin_(gr, tr("roll (z)"),
                                             tr("The z rotation of the projector's direction "
                                                "- turn left and turn right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinRoll_->setSuffix(" " + tr("°"));

                // ------- camera settings --------

                cameraGroup_ = gr = new GroupWidget(tr("virtual camera settings"), this);
                lv->addWidget(gr);
                gr->setExpanded(false);

                label = new QLabel(tr("view"), this);
                gr->addWidget(label);

                spinCamWidth_ = createSpin_(gr, tr("width"),
                                             tr("Camera's horizontal resolution in pixels"),
                                             1024, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinCamWidth_->setSuffix(" " + tr("px"));

                spinCamHeight_ = createSpin_(gr, tr("height"),
                                             tr("Camera's vertical resolution in pixels"),
                                             768, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinCamHeight_->setSuffix(" " + tr("px"));

                spinCamFov_ = createDoubleSpin_(gr, tr("field of view"),
                                             tr("Camera's (vertical) view angle in degree"),
                                             60, 0.1, 1, 179, SLOT(updateProjectorSettings_()));
                spinCamFov_->setSuffix(" " + tr("°"));

                spinCamZNear_ = createDoubleSpin_(gr, tr("near plane"),
                                             tr("The distance to the near plane in the camera frustum - "
                                                "normally no change is needed"),
                                             0.01, 0.001, 0.00001, 1000000,
                                             SLOT(updateProjectorSettings_()));

                spinCamZFar_ = createDoubleSpin_(gr, tr("far plane"),
                                             tr("The distance to the far plane in the camera frustum - "
                                                "normally no change is needed"),
                                             0.01, 0.001, 0.00001, 1000000,
                                             SLOT(updateProjectorSettings_()));

                label = new QLabel(tr("orientation"), this);
                gr->addWidget(label);

                spinCamPitch_ = createDoubleSpin_(gr, tr("pitch (x)"),
                                             tr("The x rotation of the camera's direction "
                                                "- up and down"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamPitch_->setSuffix(" " + tr("°"));

                spinCamYaw_ = createDoubleSpin_(gr, tr("yaw (y)"),
                                             tr("The y rotation of the camera's direction "
                                                "- left and right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamYaw_->setSuffix(" " + tr("°"));

                spinCamRoll_ = createDoubleSpin_(gr, tr("roll (z)"),
                                             tr("The z rotation of the camera's direction "
                                                "- turn left and turn right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamRoll_->setSuffix(" " + tr("°"));

                label = new QLabel(tr("position"), this);
                gr->addWidget(label);

                spinCamX_ = createDoubleSpin_(gr, tr("x"),
                                             tr("Camera's position on the x axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                spinCamY_ = createDoubleSpin_(gr, tr("y"),
                                             tr("Camera's position on the y axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                spinCamZ_ = createDoubleSpin_(gr, tr("z"),
                                             tr("Camera's position on the z axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                // ------- blend settings ------

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    label = new QLabel(tr("Blend method"), this);
                    lh2->addWidget(label);

                    spinBlendMethod_ = new SpinBox(this);
                    lh2->addWidget(spinBlendMethod_);
                    spinBlendMethod_->setRange(0,1);
                    connect(spinBlendMethod_, SIGNAL(valueChanged(int)),
                            this, SLOT(updateProjectorSettings_()));

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    label = new QLabel(tr("Blend margin"), this);
                    lh2->addWidget(label);

                    spinBlendMargin_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinBlendMargin_);
                    spinBlendMargin_->setRange(0,1);
                    spinBlendMargin_->setSingleStep(0.025);
                    connect(spinBlendMargin_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateProjectorSettings_()));

                // ------- overlap area --------

                /*areaGroup_ = gr = new GroupWidget(tr("overlap area blending"), this);
                lv->addWidget(gr);
                gr->setExpanded(true);
                */

                areaEdit_ = new OverlapAreaEditWidget(this);
                areaEdit_->setMinimumSize(160,90);
                lv->addWidget(areaEdit_);
                areaEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                connect(areaEdit_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));

                //areaGroup_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

                auto cb = new QCheckBox(tr("show tesselation"), this);
                lv->addWidget(cb);
                connect(cb, &QCheckBox::clicked, [=](bool c)
                {
                    areaEdit_->setShowTesselation(c);
                });


            //lv0->addStretch(2);

        // --- preview display ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            display_ = new DomePreviewWidget(this);
            display_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            lv->addWidget(display_);
            connect(display_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));

            display_->setTextureCallback(
                        std::bind(&ProjectorSetupDialog::createTexture_, this, std::placeholders::_1));

            // --- display settings ---

            QWidget * projectionStuff = new QWidget(this);
            lv->addWidget(projectionStuff);

            lh2 = new QHBoxLayout(projectionStuff);
            lh2->setMargin(0);

                comboView_ = new QComboBox(this);
                lh2->addWidget(comboView_);
                comboView_->setStatusTip(tr("Selects the projection type of the preview window"));
                comboView_->addItem(tr("orthographic"), Basic3DWidget::RM_DIRECT_ORTHO);
                comboView_->addItem(tr("perspective"), Basic3DWidget::RM_DIRECT);
                comboView_->addItem(tr("fulldome cubemap"), Basic3DWidget::RM_FULLDOME_CUBE);
                comboView_->setCurrentIndex(1);
                connect(comboView_, SIGNAL(currentIndexChanged(int)),
                        this, SLOT(changeView_()));

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_front.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_FRONT); });

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_back.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_BACK); });

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_left.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_LEFT); });

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_right.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_RIGHT); });

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_top.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_TOP); });

                tbut = new QToolButton(this);
                lh2->addWidget(tbut);
                tbut->setIcon(QIcon(":/icon/view_bottom.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_BOTTOM); });

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                cb = new QCheckBox(tr("show coordinates"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowGrid());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowGrid(bool)));

                cb = new QCheckBox(tr("show dome"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowDome());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowDome(bool)));

                cb = new QCheckBox(tr("show rays"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowRays());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowRays(bool)));

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                cb = new QCheckBox(tr("highlight current projector"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowHighlight());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowHighlight(bool)));

                cb = new QCheckBox(tr("show content"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowTexture());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowTexture(bool)));


            // --- dome settings ---

            frame = new QFrame(this);
            lv->addWidget(frame);
            frame->setFrameStyle(QFrame::Raised | QFrame::Panel);

            lv = new QVBoxLayout(frame);
            lv->setMargin(2);

                gr = new GroupWidget(tr("dome settings"), this);
                lv->addWidget(gr);
                gr->setExpanded(false);

                label = new QLabel(tr("dome settings"), this);
                gr->addWidget(label);

                editDomeName_ = createEdit_(gr, tr("name"),
                                            tr("Some name to identify the dome/planetarium"),
                                        "planetarium", SLOT(updateDomeName_()));

                spinDomeRad_ = createDoubleSpin_(gr, tr("radius"),
                                             tr("The dome radius in meters - "
                                                "messured at the 180° horizon"),
                                             10, 0.5, 0.1, 1000, SLOT(updateDomeSettings_()));
                spinDomeRad_->setSuffix(" " + tr("m"));

                spinDomeCover_ = createDoubleSpin_(gr, tr("coverage"),
                                             tr("The coverage of the dome in degree - "
                                                "180 = half-sphere"),
                                             180, 1, 1, 360, SLOT(updateDomeSettings_()));
                spinDomeCover_->setSuffix(" " + tr("°"));

                spinDomeTiltX_ = createDoubleSpin_(gr, tr("tilt x"),
                                             tr("The tilt in degree on the x axis"),
                                             0, 1, -360, 360, SLOT(updateDomeSettings_()));
                spinDomeTiltX_->setSuffix(" " + tr("°"));

                spinDomeTiltZ_ = createDoubleSpin_(gr, tr("tilt z"),
                                             tr("The tilt in degree on the z axis"),
                                             0, 1, -360, 360, SLOT(updateDomeSettings_()));
                spinDomeTiltZ_->setSuffix(" " + tr("°"));


    // link groups so only one is visible at a time
    // and also change display mode
    connect(projectorGroup_, &GroupWidget::expanded, [=]()
    {
        cameraGroup_->collapse();
        projectionStuff->setEnabled(true);
        display_->setShowCurrentCamera(false);
    });
    connect(projectorGroup_, &GroupWidget::collapsed, [=]()
    {
        cameraGroup_->expand();
        projectionStuff->setEnabled(false);
        display_->setShowCurrentCamera(true);
    });
    connect(cameraGroup_, &GroupWidget::expanded, [=]()
    {
        projectorGroup_->collapse();
        projectionStuff->setEnabled(false);
        display_->setShowCurrentCamera(true);
    });
    connect(cameraGroup_, &GroupWidget::collapsed, [=]()
    {
        projectorGroup_->expand();
        projectionStuff->setEnabled(true);
        display_->setShowCurrentCamera(false);
    });
}

QLineEdit * ProjectorSetupDialog::createEdit_(GroupWidget * group,
                    const QString& desc, const QString& statusTip,
                    const QString& value,
                    const char * slot,
                    bool readOnly)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    group->addWidget(w);

        auto lh = new QHBoxLayout(w);
        lh->setMargin(1);

            auto label = new QLabel(desc, this);
            label->setStatusTip(statusTip);
            lh->addWidget(label);

            auto edit = new QLineEdit(value, this);
            lh->addWidget(edit);
            edit->setReadOnly(readOnly);
            edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

            if (slot)
                connect(edit, SIGNAL(textChanged(QString)), this, slot);
    return edit;
}

SpinBox * ProjectorSetupDialog::createSpin_(GroupWidget * group,
                    const QString& desc, const QString& statusTip,
                    int value, int smallstep, int minv, int maxv,
                    const char * slot)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    group->addWidget(w);

        auto lh = new QHBoxLayout(w);
        lh->setMargin(1);

            auto label = new QLabel(desc, this);
            label->setStatusTip(statusTip);
            lh->addWidget(label);

            auto spin = new SpinBox(this);
            spin->setSingleStep(smallstep);
            spin->setRange(minv, maxv);
            spin->setStatusTip(statusTip);
            spin->setValue(value);
            lh->addWidget(spin);

            if (slot)
                connect(spin, SIGNAL(valueChanged(int)), this, slot);
    return spin;
}

DoubleSpinBox * ProjectorSetupDialog::createDoubleSpin_(GroupWidget * group,
                    const QString& desc, const QString& statusTip,
                    double value, double smallstep, double minv, double maxv,
                    const char * slot)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    group->addWidget(w);

        auto lh = new QHBoxLayout(w);
        lh->setMargin(1);

            auto label = new QLabel(desc, this);
            label->setStatusTip(statusTip);
            lh->addWidget(label);

            auto spin = new DoubleSpinBox(this);
            spin->setDecimals(7);
            spin->setSingleStep(smallstep);
            spin->setRange(minv, maxv);
            spin->setStatusTip(statusTip);
            spin->setValue(value);
            lh->addWidget(spin);

            if (slot)
                connect(spin, SIGNAL(valueChanged(double)), this, slot);
    return spin;
}

void ProjectorSetupDialog::closeEvent(QCloseEvent * e)
{
    if (!saveToClose_())
    {
        e->ignore();
        return;
    }

    // release gl resources from testrenderer
    if (testRenderer_)
    {
        // XXX this one is a bit hacky
        // we need a current context, and don't have it anywhere
        // in this dialog... so pull it in here
        if (display_ && display_->context())
            display_->context()->makeCurrent();
        // this will throw an exception if context is not ready
        try { testRenderer_->releaseGl(); } catch(...) { }
    }

    if (display_->isGlInitialized() || areaEdit_->isGlInitialized())
    {
        if (display_->isGlInitialized())
            display_->shutDownGL();

        if (areaEdit_->isGlInitialized())
            areaEdit_->shutDownGL();

        closeRequest_ = true;
        e->ignore();
    }
    else QDialog::closeEvent(e);
}

void ProjectorSetupDialog::onGlReleased_()
{
    if (closeRequest_)
        close();
}


void ProjectorSetupDialog::changeView_()
{
    if (comboView_->currentIndex() < 0)
        return;

    Basic3DWidget::RenderMode rm = (Basic3DWidget::RenderMode)
            comboView_->itemData(comboView_->currentIndex()).toInt();

    if (rm == Basic3DWidget::RM_DIRECT_ORTHO)
        display_->viewSetOrthoScale(domeSettings_->radius());

    display_->setRenderMode(rm);
}

void ProjectorSetupDialog::setViewDirection(int dir)
{
    Float distance = domeSettings_->radius() * 2.0;
    if (display_->isVisible())
        distance *= display_->height() / display_->width();
    if (dir == Basic3DWidget::VD_BOTTOM
        && display_->renderMode() == Basic3DWidget::RM_FULLDOME_CUBE)
        distance = 0;

    display_->viewSet((Basic3DWidget::ViewDirection)dir, distance);
}

void ProjectorSetupDialog::updateDomeSettings_()
{
    domeSettings_->setName(editDomeName_->text());
    domeSettings_->setRadius(spinDomeRad_->value());
    domeSettings_->setCoverage(spinDomeCover_->value());
    domeSettings_->setTiltX(spinDomeTiltX_->value());
    domeSettings_->setTiltZ(spinDomeTiltZ_->value());

    // update system settings as well
    settings_->setDomeSettings(*domeSettings_);

    updateWindowTitle_();

    updateDisplay_();
}

void ProjectorSetupDialog::updateDomeWidgets_()
{
    editDomeName_->setText(domeSettings_->name());
    spinDomeRad_->setValue(domeSettings_->radius());
    spinDomeCover_->setValue(domeSettings_->coverage());
    spinDomeTiltX_->setValue(domeSettings_->tiltX());
    spinDomeTiltZ_->setValue(domeSettings_->tiltZ());
}

void ProjectorSetupDialog::updateProjectorName_()
{
    projectorSettings_->setName(editName_->text());

    const int idx = comboProj_->currentIndex();
    if (idx >= 0 && idx < (int)settings_->numProjectors())
    {
        settings_->setProjectorSettings(idx, *projectorSettings_);

        // update name in combobox
        comboProj_->setItemText(idx, projectorSettings_->name());
    }

    updateWindowTitle_();
}

void ProjectorSetupDialog::updateDomeName_()
{
    domeSettings_->setName(editDomeName_->text());

    settings_->setDomeSettings(*domeSettings_);

    updateWindowTitle_();
}

void ProjectorSetupDialog::updateProjectorSettings_()
{
    projectorSettings_->setName(editName_->text());
    projectorSettings_->setWidth(spinWidth_->value());
    projectorSettings_->setHeight(spinHeight_->value());
    projectorSettings_->setFov(spinFov_->value());
#ifndef MO_DISABLE_PROJECTOR_LENS_RADIUS
    projectorSettings_->setLensRadius(spinLensRad_->value() / 100);
#endif
    projectorSettings_->setDistance(spinDist_->value() / 100);
    projectorSettings_->setLatitude(spinLat_->value());
    projectorSettings_->setLongitude(spinLong_->value());
    projectorSettings_->setPitch(spinPitch_->value());
    projectorSettings_->setYaw(spinYaw_->value());
    projectorSettings_->setRoll(spinRoll_->value());

    cameraSettings_->setWidth(spinCamWidth_->value());
    cameraSettings_->setHeight(spinCamHeight_->value());
    cameraSettings_->setFov(spinCamFov_->value());
    cameraSettings_->setPosX(spinCamX_->value());
    cameraSettings_->setPosY(spinCamY_->value());
    cameraSettings_->setPosZ(spinCamZ_->value());
    cameraSettings_->setPitch(spinCamPitch_->value());
    cameraSettings_->setYaw(spinCamYaw_->value());
    cameraSettings_->setRoll(spinCamRoll_->value());
    cameraSettings_->setZNear(spinCamZNear_->value());
    cameraSettings_->setZFar(spinCamZFar_->value());

    // update system settings as well

    settings_->setBlendMethod(spinBlendMethod_->value());
    settings_->setBlendMargin(spinBlendMargin_->value());

    int idx = comboProj_->currentIndex();
    if (idx >= 0 && idx < (int)settings_->numProjectors())
    {
        settings_->setProjectorSettings(idx, *projectorSettings_);
        settings_->setCameraSettings(idx, *cameraSettings_);
        updateWindowTitle_();
    }

    settings_->calculateOverlapAreas();

    updateDisplay_();
}

void ProjectorSetupDialog::updateDisplay_()
{
    if (display_->renderMode() == Basic3DWidget::RM_DIRECT_ORTHO)
        display_->viewSetOrthoScale(domeSettings_->radius());

    const int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= (int)settings_->numProjectors())
        return;

    display_->setProjectionSettings(*settings_, idx);

    areaEdit_->setSettings(*settings_, idx);
}

void ProjectorSetupDialog::updateProjectorWidgets_()
{
    editId_->setText( QString::number(projectorSettings_->id()) );
    editName_->setText( projectorSettings_->name() );
    spinWidth_->setValue( projectorSettings_->width() );
    spinHeight_->setValue( projectorSettings_->height() );
    spinFov_->setValue( projectorSettings_->fov() );
#ifndef MO_DISABLE_PROJECTOR_LENS_RADIUS
    spinLensRad_->setValue( projectorSettings_->lensRadius() );
#endif
    spinDist_->setValue( projectorSettings_->distance() );
    spinLat_->setValue( projectorSettings_->latitude() );
    spinLong_->setValue( projectorSettings_->longitude() );
    spinPitch_->setValue( projectorSettings_->pitch() );
    spinYaw_->setValue( projectorSettings_->yaw() );
    spinRoll_->setValue( projectorSettings_->roll() );

    spinCamWidth_->setValue( cameraSettings_->width() );
    spinCamHeight_->setValue( cameraSettings_->height() );
    spinCamFov_->setValue( cameraSettings_->fov() );
    spinCamX_->setValue( cameraSettings_->posX() );
    spinCamY_->setValue( cameraSettings_->posY() );
    spinCamZ_->setValue( cameraSettings_->posZ() );
    spinCamPitch_->setValue( cameraSettings_->pitch() );
    spinCamYaw_->setValue( cameraSettings_->yaw() );
    spinCamRoll_->setValue( cameraSettings_->roll() );
    spinCamZNear_->setValue( cameraSettings_->zNear() );
    spinCamZFar_->setValue( cameraSettings_->zFar() );

    spinBlendMethod_->setValue( settings_->blendMethod() );
    spinBlendMargin_->setValue( settings_->blendMargin() );
}

void ProjectorSetupDialog::projectorSelected_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= (int)settings_->numProjectors())
        return;

    *projectorSettings_ = settings_->projectorSettings(idx);
    *cameraSettings_ = settings_->cameraSettings(idx);
    updateProjectorWidgets_();
    updateDisplay_();
    updateActions_();
}

void ProjectorSetupDialog::newProjector_()
{
    settings_->appendProjector(ProjectorSettings());
    *projectorSettings_ = settings_->projectorSettings(settings_->numProjectors()-1);
    updateProjectorList_();

    comboProj_->setCurrentIndex(settings_->numProjectors()-1);
    updateActions_();
}

void ProjectorSetupDialog::duplicateProjector_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= (int)settings_->numProjectors())
        return;

    settings_->appendProjector(settings_->projectorSettings(idx));
    settings_->setCameraSettings(settings_->numProjectors()-1,
                                 settings_->cameraSettings(idx));
    *projectorSettings_ = settings_->projectorSettings(settings_->numProjectors()-1);
    *cameraSettings_ = settings_->cameraSettings(settings_->numProjectors()-1);
    updateProjectorList_();

    comboProj_->setCurrentIndex(settings_->numProjectors()-1);
    updateActions_();
}

void ProjectorSetupDialog::deleteProjector_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= (int)settings_->numProjectors())
        return;

    settings_->removeProjector(idx);
    settings_->calculateOverlapAreas();

    updateProjectorList_();
    updateActions_();
}

void ProjectorSetupDialog::updateProjectorList_()
{
    // store index
    const int idx = comboProj_->currentIndex();

    comboProj_->clear();

    for (uint i=0; i<settings_->numProjectors(); ++i)
        comboProj_->addItem(settings_->projectorSettings(i).name());

    tbRemove_->setEnabled(settings_->numProjectors() > 1);

    // restore index
    if (idx >= 0 && idx < (int)settings_->numProjectors())
    {
        comboProj_->setCurrentIndex(idx);
    }

    updateActions_();
}

void ProjectorSetupDialog::clearPreset_()
{
    if (!saveToClear_())
        return;

    settings_->clear();
    settings_->appendProjector(ProjectorSettings());
    *orgSettings_ = *settings_;
    *projectorSettings_ = settings_->projectorSettings(0);
    *cameraSettings_ = settings_->cameraSettings(0);
    *domeSettings_ = settings_->domeSettings();

    settings_->calculateOverlapAreas();

    updateDomeWidgets_();
    updateProjectorWidgets_();
    updateProjectorList_();
    updateActions_();
    updateDisplay_();
}

void ProjectorSetupDialog::loadDefault_()
{
    if (!saveToClear_())
        return;

    *settings_ = settings->getDefaultProjectionSettings();
    *orgSettings_ = *settings_;
    *projectorSettings_ = settings_->projectorSettings(0);
    *cameraSettings_ = settings_->cameraSettings(0);
    *domeSettings_ = settings_->domeSettings();
    filename_.clear();

    settings_->calculateOverlapAreas();

    updateDomeWidgets_();
    updateProjectorWidgets_();
    updateProjectorList_();
    updateActions_();
    updateDisplay_();
}

void ProjectorSetupDialog::saveDefault_()
{
    settings->setDefaultProjectionSettings(*settings_);

    // update clients
    if (serverEngine().isRunning())
        serverEngine().sendProjectionSettings();
}

bool ProjectorSetupDialog::savePresetAuto_()
{
    if (filename_.isEmpty())
        return savePresetChoose_();
    else
        return savePreset_(filename_);
}

bool ProjectorSetupDialog::savePresetChoose_()
{
    QString fn = IO::Files::getSaveFileName(IO::FT_PROJECTION_SETTINGS);
    if (!fn.isEmpty())
        return savePreset_(fn);
    return false;
}

bool ProjectorSetupDialog::savePreset_(const QString& fn)
{
    if (fn.isEmpty())
        return false;

    try
    {
        settings_->saveFile(fn);
        filename_ = fn;
        *orgSettings_ = *settings_;
        updateWindowTitle_();
        return true;
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("file i/o"),
                              tr("Could not save projection settings") + "\n" + e.what());
    }
    return false;
}

void ProjectorSetupDialog::loadPreset_()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_PROJECTION_SETTINGS);
    if (fn.isEmpty())
        return;

    try
    {
        settings_->loadFile(fn);
        *orgSettings_ = *settings_;
        filename_ = fn;
        *projectorSettings_ = settings_->projectorSettings(0);
        *cameraSettings_ = settings_->cameraSettings(0);
        *domeSettings_ = settings_->domeSettings();

        settings_->calculateOverlapAreas();

        updateProjectorList_();
        updateDomeWidgets_();
        updateProjectorWidgets_();
        updateActions_();
        updateWindowTitle_();
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("file i/o"),
                              tr("Could not load projection settings") + "\n" + e.what());
    }
}

void ProjectorSetupDialog::updateWindowTitle_()
{
    QString title = tr("projector setup");

    if (*settings_ != *orgSettings_)
        title += " *";

    if (!filename_.isEmpty())
        title += " [" + QFileInfo(filename_).fileName() + "]";

    setWindowTitle(title);
}

bool ProjectorSetupDialog::saveToClear_()
{
    bool r = saveToClose_();
    saidNoAlready_ = false;
    return r;
}

bool ProjectorSetupDialog::saveToClose_()
{
    if (saidNoAlready_ || *settings_ == *orgSettings_)
        return true;

    QMessageBox::Button res =
        QMessageBox::question(this, tr("unsaved changes"),
                              tr("There are unsaved changes, do you want to save them?\n%1")
                              .arg(filename_.isEmpty()? tr("(choose file)") :
                                                    tr("(to file %1)").arg(filename_)),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                              QMessageBox::Yes);
    if (res == QMessageBox::Cancel)
        return false;

    if (res == QMessageBox::No)
    {
        saidNoAlready_ = true;
        return true;
    }

    return savePresetAuto_();
}

void ProjectorSetupDialog::updateActions_()
{
    aPasteProjector_->setEnabled( copyOfProjectorSettings_ != 0);
    aPasteCamera_->setEnabled( copyOfCameraSettings_ != 0 );

    const int idx = comboProj_->currentIndex();
    aPrevious_->setEnabled(idx >= 1);
    aNext_->setEnabled(idx < comboProj_->count()-1);

}

void ProjectorSetupDialog::previousProjector_()
{
    const int idx = comboProj_->currentIndex();
    if (idx < 1 || idx >= comboProj_->count())
        return;
    comboProj_->setCurrentIndex(idx-1);
    updateActions_();
}

void ProjectorSetupDialog::nextProjector_()
{
    const int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= comboProj_->count()-1)
        return;
    comboProj_->setCurrentIndex(idx+1);
    updateActions_();
}

void ProjectorSetupDialog::copyProjector_()
{
    if (copyOfProjectorSettings_ == 0)
        copyOfProjectorSettings_ = new ProjectorSettings();
    *copyOfProjectorSettings_ = *projectorSettings_;

    updateActions_();
}


void ProjectorSetupDialog::copyCamera_()
{
    if (copyOfCameraSettings_ == 0)
        copyOfCameraSettings_ = new CameraSettings();
    *copyOfCameraSettings_ = *cameraSettings_;

    updateActions_();
}

void ProjectorSetupDialog::pasteProjector_()
{
    if (copyOfProjectorSettings_ == 0)
        return;

    *projectorSettings_ = *copyOfProjectorSettings_;

    int idx = comboProj_->currentIndex();
    if (idx >= 0 || idx < (int)settings_->numProjectors())
    {
        settings_->setProjectorSettings(idx, *projectorSettings_);
    }

    updateProjectorWidgets_();
    updateDisplay_();
}


void ProjectorSetupDialog::pasteCamera_()
{
    if (copyOfCameraSettings_ == 0)
        return;

    *cameraSettings_ = *copyOfCameraSettings_;

    int idx = comboProj_->currentIndex();
    if (idx >= 0 || idx < (int)settings_->numProjectors())
    {
        settings_->setCameraSettings(idx, *cameraSettings_);
    }

    updateProjectorWidgets_();
    updateDisplay_();
}

GL::Texture * ProjectorSetupDialog::createTexture_(int index)
{
#if 0
    if (!testRenderer_)
        testRenderer_ = new TestProjectionRenderer();

    testRenderer_->setSettings(*settings_);

    return testRenderer_->renderSliceTexture(index);
#elif 0

    ProjectorMapper mapper, omapper;
    mapper.setSettings(settings_->domeSettings(),
                       settings_->projectorSettings(index) );
    return mapper.renderBlendTexture(index, *settings_);

    //omapper.setSettings(settings_->domeSettings(),
    //                    settings_->projectorSettings((index+1) % settings_->numProjectors()) );
    //return mapper.renderBlendTexture(omapper);
#else

    ProjectorBlender blender(settings_);
    return blender.renderBlendTexture(index, 320);

#endif
}

} // namespace GUI
} // namespace MO
