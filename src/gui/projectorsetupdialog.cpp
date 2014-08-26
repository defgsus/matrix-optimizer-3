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

namespace MO {
namespace GUI {


ProjectorSetupDialog::ProjectorSetupDialog(QWidget *parent)
    : QDialog       (parent),
      closeRequest_ (false)
{
    setObjectName("_ProjectorSetupDialog");
    setWindowTitle(tr("projector setup"));
    setMinimumSize(640,480);

    createWidgets_();
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
                                         tr("Projector's opening angle"),
                                         60, 1, 1, 180);

            label = new QLabel(tr("position"), this);
            lv->addWidget(label);

            spinLat_ = createDoubleSpin(lv, tr("latitude"),
                                         tr("Projector's position around the dome"),
                                         0, 1, -360, 360);

            spinLong_ = createDoubleSpin(lv, tr("longitude"),
                                         tr("Projector's height"),
                                         0, 1, -90, 90);

            label = new QLabel(tr("orientation"), this);
            lv->addWidget(label);

            spinPitch_ = createDoubleSpin(lv, tr("pitch (x)"),
                                         tr("The x rotation of the Projector's direction"),
                                         0, 1, -360, 360);

            spinRoll_ = createDoubleSpin(lv, tr("roll (y)"),
                                         tr("The y rotation of the Projector's direction"),
                                         0, 1, -360, 360);

            spinYaw_ = createDoubleSpin(lv, tr("yaw (z)"),
                                         tr("The z rotation of the Projector's direction"),
                                         0, 1, -360, 360);

            lv->addStretch(2);

        // --- preview display

        lv = new QVBoxLayout();
        lh->addLayout(lv);

            display_ = new DomePreviewWidget(this);
            lv->addWidget(display_);
            connect(display_, SIGNAL(glReleased()), this, SLOT(onGlReleased_()));
}

DoubleSpinBox * ProjectorSetupDialog::createDoubleSpin(QLayout * layout,
                    const QString& desc, const QString& statusTip,
                    double value, double smallstep, double minv, double maxv)
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
            spin->setValue(value);
            spin->setSingleStep(smallstep);
            spin->setRange(minv, maxv);
            spin->setStatusTip(statusTip);
            lh->addWidget(spin);

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

} // namespace GUI
} // namespace MO
