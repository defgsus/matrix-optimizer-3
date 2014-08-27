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

#include "projectorsetupdialog.h"
#include "widget/domepreviewwidget.h"
#include "widget/spinbox.h"
#include "widget/doublespinbox.h"
#include "projection/domesettings.h"
#include "projection/projectorsettings.h"
#include "io/xmlstream.h"
#include "io/log.h"

namespace MO {
namespace GUI {


ProjectorSetupDialog::ProjectorSetupDialog(QWidget *parent)
    : QDialog       (parent),
      closeRequest_ (false),
      domeSettings_ (new DomeSettings()),
      projectorSettings_(new ProjectorSettings())
{
    setObjectName("_ProjectorSetupDialog");
    setWindowTitle(tr("projector setup"));
    setMinimumSize(760,600);

    createWidgets_();

    updateDomeSettings_();
    updateProjectorSettings_();

    setViewDirection(Basic3DWidget::VD_FRONT);
}

ProjectorSetupDialog::~ProjectorSetupDialog()
{
    delete domeSettings_;
    delete projectorSettings_;
}

void ProjectorSetupDialog::createWidgets_()
{
    auto lh = new QHBoxLayout(this);

        // --- projector setup ---

        auto frame = new QFrame(this);
        lh->addWidget(frame);
        frame->setFrameStyle(QFrame::Raised);

        auto lv = new QVBoxLayout(frame);

            // ------ projector select ------------

            comboProj_ = new QComboBox(this);
            lv->addWidget(comboProj_);


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
                                         tr("The x rotation of the Projector's direction "
                                            "- up and down"),
                                         0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
            spinPitch_->setSuffix(" " + tr("°"));

            spinYaw_ = createDoubleSpin_(lv, tr("yaw (y)"),
                                         tr("The y rotation of the Projector's direction "
                                            "- left and right"),
                                         0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
            spinYaw_->setSuffix(" " + tr("°"));

            spinRoll_ = createDoubleSpin_(lv, tr("roll (z)"),
                                         tr("The z rotation of the Projector's direction "
                                            "- turn left and turn right"),
                                         0, 0.1, -360, 360, SLOT(updateProjectorSettings_()));
            spinRoll_->setSuffix(" " + tr("°"));

            lv->addStretch(2);

            auto but = new QPushButton("debug");
            lv->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                IO::XmlStream io;
                io.startWriting("projector-setup");
                projectorSettings_->serialize(io);
                io.stopWriting();
                MO_DEBUG(io.data());

                io.startReading("projector-setup");
                io.nextSubSection();
                projectorSettings_->deserialize(io);
                io.stopReading();
            });

        // --- preview display ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            display_ = new DomePreviewWidget(this);
            display_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            lv->addWidget(display_);
            connect(display_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));

            // --- display settings ---

            auto lh2 = new QHBoxLayout();
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

                auto tbut = new QToolButton(this);
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


            // --- dome settings ---

            label = new QLabel(tr("dome settings"), this);
            lv->addWidget(label);

            spinDomeRad_ = createDoubleSpin_(lv, tr("radius"),
                                         tr("The dome radius in meters - messured at the 180° horizon"),
                                         10, 0.5, 0.1, 1000, SLOT(updateDomeSettings_()));
            spinDomeRad_->setSuffix(" " + tr("m"));

            spinDomeCover_ = createDoubleSpin_(lv, tr("coverage"),
                                         tr("The coverage of the dome in degree - 180 = half-sphere"),
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
    domeSettings_->setRadius(spinDomeRad_->value());
    domeSettings_->setCoverage(spinDomeCover_->value());
    domeSettings_->setTiltX(spinDomeTiltX_->value());
    domeSettings_->setTiltZ(spinDomeTiltZ_->value());

    if (display_->renderMode() == Basic3DWidget::RM_DIRECT_ORTHO)
        display_->viewSetOrthoScale(domeSettings_->radius());

    display_->setDomeSettings(*domeSettings_);
}

void ProjectorSetupDialog::updateProjectorName_()
{
    projectorSettings_->setName(editName_->text());
}

void ProjectorSetupDialog::updateProjectorSettings_()
{
    projectorSettings_->setWidth(spinWidth_->value());
    projectorSettings_->setHeight(spinHeight_->value());
    projectorSettings_->setFov(spinFov_->value());
    projectorSettings_->setLensRadius(spinLensRad_->value() / 100);
    projectorSettings_->setDistance(spinDist_->value() / 100);
    projectorSettings_->setLatitude(spinLat_->value());
    projectorSettings_->setLongitude(spinLong_->value());
    projectorSettings_->setPitch(spinPitch_->value());
    projectorSettings_->setRoll(spinRoll_->value());
    projectorSettings_->setYaw(spinYaw_->value());

    display_->setProjectorSettings(*projectorSettings_);
}


} // namespace GUI
} // namespace MO
