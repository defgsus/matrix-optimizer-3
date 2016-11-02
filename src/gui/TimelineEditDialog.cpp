/** @file timelineeditdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#include <QLayout>
#include <QTimer>
#include <QCheckBox>
#include <QPushButton>

#include "TimelineEditDialog.h"
#include "math/Timeline1d.h"
#include "Timeline1dRulerView.h"
#include "Timeline1dView.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {

TimelineEditDialog::TimelineEditDialog(QWidget *parent)
    : QDialog       (parent),
      tl_           (nullptr),
      autoUpdate_   (false),
      options_      (Timeline1DView::O_EnableAll)
{
    setObjectName("_TimelineEditDialog");
    setWindowTitle(tr("Timeline editor"));

    setMinimumSize(320,200);

    settings()->restoreGeometry(this);
    autoUpdate_ = settings()->value(
                objectName()+"/autoupdate", autoUpdate_).toBool();

    createWidgets_();
}

TimelineEditDialog::~TimelineEditDialog()
{
    settings()->storeGeometry(this);
    settings()->setValue(objectName()+"/autoupdate", autoUpdate_);
    if (tl_)
        tl_->releaseRef("TimelineEditDialog close");
}

void TimelineEditDialog::createWidgets_()
{
    timer_ = new QTimer(this);
    timer_->setSingleShot(false);
    // timeout for emitting a changed signal
    timer_->setInterval(300);
    connect(timer_, SIGNAL(timeout()), this, SIGNAL(timelineChanged()));

    auto lv = new QVBoxLayout(this);

        // editor
        editor_ = new Timeline1DRulerView(tl_, this);
        lv->addWidget(editor_);
        editor_->setOptions(options_);

        connect(editor_, &Timeline1DRulerView::timelineChanged, [=]()
        {
            if (autoUpdate_)
                timer_->start();
        });

        // lower area
        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // ok
            auto bok = new QPushButton(tr("ok"), this);
            lh->addWidget(bok);
            bok->setDefault(true);
            connect(bok, &QPushButton::clicked, [=]()
            {
                accept();
            });

            // cancel
            auto bcanc = new QPushButton(tr("cancel"), this);
            lh->addWidget(bcanc);
            connect(bcanc, &QPushButton::clicked, [=]()
            {
                reject();
            });

            // always update?
            auto cb = new QCheckBox(tr("always update"), this);
            lh->addWidget(cb);
            cb->setChecked(autoUpdate_);
            connect(bok, &QCheckBox::clicked, [=]()
            {
                autoUpdate_ = cb->isChecked();
            });
}


const Timeline1DView & TimelineEditDialog::editor() const
{
    return *editor_->timelineView();
}

Timeline1DView & TimelineEditDialog::editor()
{
    return *editor_->timelineView();
}

void TimelineEditDialog::setOptions(int o)
{
    options_ = o;
    editor_->setOptions(options_);
}

void TimelineEditDialog::setViewSpace(const UTIL::ViewSpace& vs)
{
    editor_->setViewSpace(vs);
}

void TimelineEditDialog::setTimeline(const MATH::Timeline1d & tl)
{
    if (!tl_)
        tl_ = new MATH::Timeline1d();

    *tl_ = tl;

    editor_->setTimeline(tl_);
}





} // namespace GUI
} // namespace MO
