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

#include "projectorsetupdialog.h"
#include "widget/domepreviewwidget.h"
#include "widget/spinbox.h"
#include "widget/doublespinbox.h"
#include "projection/domesettings.h"
#include "projection/projectorsettings.h"

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
    setMinimumSize(640,480);

    createWidgets_();

    //updateDomeSettings_();
    //updateProjectorSettings_();
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

            auto label = new QLabel(tr("projector settings"), this);
            lv->addWidget(label);

            spinFov_ = createDoubleSpin(lv, tr("field of view"),
                                         tr("Projector's projection angle (for width) in degree"),
                                         60, 1, 1, 180, SLOT(updateProjectorSettings_()));
            spinFov_->setSuffix(" " + tr("°"));

            spinLensRad_ = createDoubleSpin(lv, tr("lens radius"),
                                         tr("The radius of the projector's lens in centimeters"),
                                         0, 0.1, 0, 1000, SLOT(updateProjectorSettings_()));
            spinLensRad_->setSuffix(" " + tr(" cm"));

            label = new QLabel(tr("position"), this);
            lv->addWidget(label);

            spinRadius_ = createDoubleSpin(lv, tr("radius"),
                                         tr("Projector's position relative to the center of the dome"),
                                         10, 0.01, 0, 1000, SLOT(updateProjectorSettings_()));
            spinRadius_->setSuffix(" " + tr("m"));

            spinLat_ = createDoubleSpin(lv, tr("latitude"),
                                         tr("Projector's position around the dome"),
                                         0, 1, -360, 360, SLOT(updateProjectorSettings_()));
            spinLat_->setSuffix(" " + tr("°"));

            spinLong_ = createDoubleSpin(lv, tr("longitude"),
                                         tr("Projector's height"),
                                         0, 1, -90, 90, SLOT(updateProjectorSettings_()));
            spinLong_->setSuffix(" " + tr("°"));

            label = new QLabel(tr("orientation"), this);
            lv->addWidget(label);

            spinPitch_ = createDoubleSpin(lv, tr("pitch (x)"),
                                         tr("The x rotation of the Projector's direction "
                                            "- up and down"),
                                         0, 1, -360, 360, SLOT(updateProjectorSettings_()));
            spinPitch_->setSuffix(" " + tr("°"));

            spinYaw_ = createDoubleSpin(lv, tr("yaw (y)"),
                                         tr("The y rotation of the Projector's direction "
                                            "- left and right"),
                                         0, 1, -360, 360, SLOT(updateProjectorSettings_()));
            spinYaw_->setSuffix(" " + tr("°"));

            spinRoll_ = createDoubleSpin(lv, tr("roll (z)"),
                                         tr("The z rotation of the Projector's direction "
                                            "- turn left and turn right"),
                                         0, 1, -360, 360, SLOT(updateProjectorSettings_()));
            spinRoll_->setSuffix(" " + tr("°"));

            lv->addStretch(2);

        // --- preview display ---

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            display_ = new DomePreviewWidget(this);
            display_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            lv->addWidget(display_);
            connect(display_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));

            // --- dome settings ---

            label = new QLabel(tr("dome settings"), this);
            lv->addWidget(label);

            spinDomeRad_ = createDoubleSpin(lv, tr("radius"),
                                         tr("The dome radius in meters - messured at the 180° horizon"),
                                         10, 0.5, 0.1, 1000, SLOT(updateDomeSettings_()));
            spinDomeRad_->setSuffix(" " + tr("m"));

            spinDomeCover_ = createDoubleSpin(lv, tr("coverage"),
                                         tr("The coverage of the dome in degree - 180 = half-sphere"),
                                         180, 1, 1, 360, SLOT(updateDomeSettings_()));
            spinDomeCover_->setSuffix(" " + tr("°"));

            spinDomeTiltX_ = createDoubleSpin(lv, tr("tilt x"),
                                         tr("The tilt in degree on the x axis"),
                                         0, 1, -360, 360, SLOT(updateDomeSettings_()));
            spinDomeTiltX_->setSuffix(" " + tr("°"));

            spinDomeTiltZ_ = createDoubleSpin(lv, tr("tilt z"),
                                         tr("The tilt in degree on the z axis"),
                                         0, 1, -360, 360, SLOT(updateDomeSettings_()));
            spinDomeTiltZ_->setSuffix(" " + tr("°"));


}

DoubleSpinBox * ProjectorSetupDialog::createDoubleSpin(QLayout * layout,
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
            spin->setDecimals(5);
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

void ProjectorSetupDialog::updateDomeSettings_()
{
    domeSettings_->setRadius(spinDomeRad_->value());
    domeSettings_->setCoverage(spinDomeCover_->value());
    domeSettings_->setTiltX(spinDomeTiltX_->value());
    domeSettings_->setTiltZ(spinDomeTiltZ_->value());

    display_->setDomeSettings(*domeSettings_);
}

void ProjectorSetupDialog::updateProjectorSettings_()
{
    projectorSettings_->setFov(spinFov_->value());
    projectorSettings_->setLensRadius(spinLensRad_->value() / 100);
    projectorSettings_->setRadius(spinRadius_->value());
    projectorSettings_->setLatitude(spinLat_->value());
    projectorSettings_->setLongitude(spinLong_->value());
    projectorSettings_->setPitch(spinPitch_->value());
    projectorSettings_->setRoll(spinRoll_->value());
    projectorSettings_->setYaw(spinYaw_->value());

    display_->setProjectorSettings(*projectorSettings_);
}

} // namespace GUI
} // namespace MO
