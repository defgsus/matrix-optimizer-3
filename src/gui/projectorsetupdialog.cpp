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
#include "projection/projectionsystemsettings.h"
#include "io/xmlstream.h"
#include "io/log.h"
#include "io/files.h"

#include "projection/projectormapper.h"

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
      cameraSettings_   (new CameraSettings())
{
    setObjectName("_ProjectorSetupDialog");
    setMinimumSize(760,600);

    createWidgets_();
    createMenu_();

    // init default settings
    clearPreset_();
    /*
    settings_->appendProjector(ProjectorSettings());
    *orgSettings_ = *settings_;
    *projectorSettings_ = settings_->projectorSettings(0);

    updateWindowTitle_();

    updateProjectorList_();
    updateDomeWidgets_();
    updateProjectorWidgets_();
    */
    setViewDirection(Basic3DWidget::VD_FRONT);
}

ProjectorSetupDialog::~ProjectorSetupDialog()
{
    delete cameraSettings_;
    delete projectorSettings_;
    delete domeSettings_;
    delete orgSettings_;
    delete settings_;
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

                // --------- projector settings --------------

                auto label = new QLabel(tr("projector settings"), this);
                lv->addWidget(label);

                editName_ = createEdit_(lv, tr("name"),
                                            tr("Some name to identify the projector"),
                                        "projector", SLOT(updateProjectorName_()));

                spinWidth_ = createSpin_(lv, tr("width"),
                                             tr("Projector's horizontal resolution in pixels"),
                                             1024, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinWidth_->setSuffix(" " + tr("px"));

                spinHeight_ = createSpin_(lv, tr("height"),
                                             tr("Projector's vertical resolution in pixels"),
                                             768, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinHeight_->setSuffix(" " + tr("px"));

                spinFov_ = createDoubleSpin_(lv, tr("field of view"),
                                             tr("Projector's projection (horizontal) angle in degree"),
                                             60, 1, 1, 180, SLOT(updateProjectorSettings_()));
                spinFov_->setSuffix(" " + tr("°"));

                spinLensRad_ = createDoubleSpin_(lv, tr("lens radius"),
                                             tr("The radius of the projector's lens in centimeters"),
                                             0, 0.1, 0, 1000, SLOT(updateProjectorSettings_()));
                spinLensRad_->setSuffix(" " + tr(" cm"));

                label = new QLabel(tr("position"), this);
                lv->addWidget(label);

                spinDist_ = createDoubleSpin_(lv, tr("distance"),
                                             tr("Projector's distance to the dome periphery "
                                                "in centimeters - "
                                                "negative is inside, positive is outside of dome"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));
                spinDist_->setSuffix(" " + tr("cm"));

                spinLat_ = createDoubleSpin_(lv, tr("latitude"),
                                             tr("Projector's position around the dome"),
                                             0, 1, -360, 360, SLOT(updateProjectorSettings_()));
                spinLat_->setSuffix(" " + tr("°"));

                spinLong_ = createDoubleSpin_(lv, tr("longitude"),
                                             tr("Projector's height"),
                                             0, 1, -90, 90, SLOT(updateProjectorSettings_()));
                spinLong_->setSuffix(" " + tr("°"));

                label = new QLabel(tr("orientation"), this);
                lv->addWidget(label);

                spinPitch_ = createDoubleSpin_(lv, tr("pitch (x)"),
                                             tr("The x rotation of the projector's direction "
                                                "- up and down"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinPitch_->setSuffix(" " + tr("°"));

                spinYaw_ = createDoubleSpin_(lv, tr("yaw (y)"),
                                             tr("The y rotation of the projector's direction "
                                                "- left and right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinYaw_->setSuffix(" " + tr("°"));

                spinRoll_ = createDoubleSpin_(lv, tr("roll (z)"),
                                             tr("The z rotation of the projector's direction "
                                                "- turn left and turn right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinRoll_->setSuffix(" " + tr("°"));

                // ------- camera settings --------

                label = new QLabel(tr("virtual camera settings"), this);
                lv->addWidget(label);

                spinCamWidth_ = createSpin_(lv, tr("width"),
                                             tr("Camera's horizontal resolution in pixels"),
                                             1024, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinCamWidth_->setSuffix(" " + tr("px"));

                spinCamHeight_ = createSpin_(lv, tr("height"),
                                             tr("Camera's vertical resolution in pixels"),
                                             768, 1, 1, 8192, SLOT(updateProjectorSettings_()));
                spinCamHeight_->setSuffix(" " + tr("px"));

                spinCamFov_ = createDoubleSpin_(lv, tr("field of view"),
                                             tr("Camera's (horizontal) view angle in degree"),
                                             60, 1, 1, 180, SLOT(updateProjectorSettings_()));
                spinCamFov_->setSuffix(" " + tr("°"));

                spinCamZNear_ = createDoubleSpin_(lv, tr("near plane"),
                                             tr("The distance to the near plane in the camera frustum - "
                                                "normally no change is needed"),
                                             0.01, 0.001, 0.00001, 1000000,
                                             SLOT(updateProjectorSettings_()));

                spinCamZFar_ = createDoubleSpin_(lv, tr("far plane"),
                                             tr("The distance to the far plane in the camera frustum - "
                                                "normally no change is needed"),
                                             0.01, 0.001, 0.00001, 1000000,
                                             SLOT(updateProjectorSettings_()));

                label = new QLabel(tr("position"), this);
                lv->addWidget(label);

                spinCamX_ = createDoubleSpin_(lv, tr("x"),
                                             tr("Camera's position on the x axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                spinCamY_ = createDoubleSpin_(lv, tr("y"),
                                             tr("Camera's position on the y axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                spinCamZ_ = createDoubleSpin_(lv, tr("z"),
                                             tr("Camera's position on the z axis in graphic units - "
                                                "normally zero"),
                                             0, 0.1, -100000, 100000, SLOT(updateProjectorSettings_()));

                label = new QLabel(tr("orientation"), this);
                lv->addWidget(label);

                spinCamPitch_ = createDoubleSpin_(lv, tr("pitch (x)"),
                                             tr("The x rotation of the camera's direction "
                                                "- up and down"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamPitch_->setSuffix(" " + tr("°"));

                spinCamYaw_ = createDoubleSpin_(lv, tr("yaw (y)"),
                                             tr("The y rotation of the camera's direction "
                                                "- left and right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamYaw_->setSuffix(" " + tr("°"));

                spinCamRoll_ = createDoubleSpin_(lv, tr("roll (z)"),
                                             tr("The z rotation of the camera's direction "
                                                "- turn left and turn right"),
                                             0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
                spinCamRoll_->setSuffix(" " + tr("°"));

            lv0->addStretch(2);

        // --- preview display ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            display_ = new DomePreviewWidget(this);
            display_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            lv->addWidget(display_);
            connect(display_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));

            // --- display settings ---

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

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

                auto cb = new QCheckBox(tr("show coordinates"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowGrid());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowGrid(bool)));

                cb = new QCheckBox(tr("show dome"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowDome());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowDome(bool)));

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                cb = new QCheckBox(tr("show current camera view"), this);
                lh2->addWidget(cb);
                cb->setChecked(display_->getShowCurrentCamera());
                connect(cb, SIGNAL(clicked(bool)), display_, SLOT(setShowCurrentCamera(bool)));

            // --- dome settings ---

            frame = new QFrame(this);
            lv->addWidget(frame);
            frame->setFrameStyle(QFrame::Raised | QFrame::Panel);

            lv = new QVBoxLayout(frame);

                label = new QLabel(tr("dome settings"), this);
                lv->addWidget(label);

                editDomeName_ = createEdit_(lv, tr("name"),
                                            tr("Some name to identify the dome/planetarium"),
                                        "planetarium", SLOT(updateDomeName_()));

                spinDomeRad_ = createDoubleSpin_(lv, tr("radius"),
                                             tr("The dome radius in meters - "
                                                "messured at the 180° horizon"),
                                             10, 0.5, 0.1, 1000, SLOT(updateDomeSettings_()));
                spinDomeRad_->setSuffix(" " + tr("m"));

                spinDomeCover_ = createDoubleSpin_(lv, tr("coverage"),
                                             tr("The coverage of the dome in degree - "
                                                "180 = half-sphere"),
                                             180, 1, 1, 360, SLOT(updateDomeSettings_()));
                spinDomeCover_->setSuffix(" " + tr("°"));

                spinDomeTiltX_ = createDoubleSpin_(lv, tr("tilt x"),
                                             tr("The tilt in degree on the x axis"),
                                             0, 1, -360, 360, SLOT(updateDomeSettings_()));
                spinDomeTiltX_->setSuffix(" " + tr("°"));

                spinDomeTiltZ_ = createDoubleSpin_(lv, tr("tilt z"),
                                             tr("The tilt in degree on the z axis"),
                                             0, 1, -360, 360, SLOT(updateDomeSettings_()));
                spinDomeTiltZ_->setSuffix(" " + tr("°"));


}

QLineEdit * ProjectorSetupDialog::createEdit_(QLayout * layout,
                    const QString& desc, const QString& statusTip,
                    const QString& value,
                    const char * slot)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    layout->addWidget(w);

        auto lh = new QHBoxLayout(w);
        lh->setMargin(1);

            auto label = new QLabel(desc, this);
            label->setStatusTip(statusTip);
            lh->addWidget(label);

            auto edit = new QLineEdit(value, this);
            lh->addWidget(edit);

            if (slot)
                connect(edit, SIGNAL(textChanged(QString)), this, slot);
    return edit;
}

SpinBox * ProjectorSetupDialog::createSpin_(
                    QLayout * layout,
                    const QString& desc, const QString& statusTip,
                    int value, int smallstep, int minv, int maxv,
                    const char * slot)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    layout->addWidget(w);

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

DoubleSpinBox * ProjectorSetupDialog::createDoubleSpin_(QLayout * layout,
                    const QString& desc, const QString& statusTip,
                    double value, double smallstep, double minv, double maxv,
                    const char * slot)
{
    auto w = new QWidget(this);
    w->setAutoFillBackground(true);
    QPalette p(w->palette());
    p.setColor(QPalette::Window, p.color(QPalette::Window).darker(110));
    w->setPalette(p);
    layout->addWidget(w);

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

    if (display_->isGlInitialized())
    {
        display_->shutDownGL();
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
    Float distance = domeSettings_->radius() * 2;
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
    if (idx >= 0 && idx < settings_->numProjectors())
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
    projectorSettings_->setLensRadius(spinLensRad_->value() / 100);
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
    int idx = comboProj_->currentIndex();
    if (idx >= 0 && idx < settings_->numProjectors())
    {
        settings_->setProjectorSettings(idx, *projectorSettings_);
        settings_->setCameraSettings(idx, *cameraSettings_);
        updateWindowTitle_();
    }

    ProjectorMapper m;
    m.setSettings(*domeSettings_, *projectorSettings_);
    //m.findCenterProjection();

    updateDisplay_();
}

void ProjectorSetupDialog::updateDisplay_()
{
    if (display_->renderMode() == Basic3DWidget::RM_DIRECT_ORTHO)
        display_->viewSetOrthoScale(domeSettings_->radius());

    display_->setProjectionSettings(*settings_, comboProj_->currentIndex());
}

void ProjectorSetupDialog::updateProjectorWidgets_()
{
    editName_->setText( projectorSettings_->name() );
    spinWidth_->setValue( projectorSettings_->width() );
    spinHeight_->setValue( projectorSettings_->height() );
    spinFov_->setValue( projectorSettings_->fov() );
    spinLensRad_->setValue( projectorSettings_->lensRadius() );
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
}

void ProjectorSetupDialog::projectorSelected_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= settings_->numProjectors())
        return;

    *projectorSettings_ = settings_->projectorSettings(idx);
    *cameraSettings_ = settings_->cameraSettings(idx);
    updateProjectorWidgets_();
    updateDisplay_();
}

void ProjectorSetupDialog::newProjector_()
{
    settings_->appendProjector(ProjectorSettings());
    *projectorSettings_ = settings_->projectorSettings(settings_->numProjectors()-1);
    updateProjectorList_();

    comboProj_->setCurrentIndex(settings_->numProjectors()-1);
}

void ProjectorSetupDialog::duplicateProjector_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= settings_->numProjectors())
        return;

    settings_->appendProjector(settings_->projectorSettings(idx));
    settings_->setCameraSettings(settings_->numProjectors()-1,
                                 settings_->cameraSettings(idx));
    *projectorSettings_ = settings_->projectorSettings(settings_->numProjectors()-1);
    *cameraSettings_ = settings_->cameraSettings(settings_->numProjectors()-1);
    updateProjectorList_();

    comboProj_->setCurrentIndex(settings_->numProjectors()-1);
}

void ProjectorSetupDialog::deleteProjector_()
{
    int idx = comboProj_->currentIndex();
    if (idx < 0 || idx >= settings_->numProjectors())
        return;

    settings_->removeProjector(idx);
    updateProjectorList_();
}

void ProjectorSetupDialog::updateProjectorList_()
{
    // store index
    const int idx = comboProj_->currentIndex();

    comboProj_->clear();

    for (int i=0; i<settings_->numProjectors(); ++i)
        comboProj_->addItem(settings_->projectorSettings(i).name());

    tbRemove_->setEnabled(settings_->numProjectors() > 1);

    // restore index
    if (idx >= 0 && idx < settings_->numProjectors())
    {
        comboProj_->setCurrentIndex(idx);
    }

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

    updateDomeWidgets_();
    updateProjectorWidgets_();
    updateProjectorList_();
    updateDisplay_();
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

        updateProjectorList_();
        updateDomeSettings_();
        updateProjectorSettings_();
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


} // namespace GUI
} // namespace MO
