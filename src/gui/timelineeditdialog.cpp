/** @file timelineeditdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#include <QLayout>
#include <QTimer>

#include "timelineeditdialog.h"
#include "math/timeline1d.h"
#include "timeline1drulerview.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

TimelineEditDialog::TimelineEditDialog(QWidget *parent)
    : QDialog       (parent),
      tl_           (0)
{
    setObjectName("_TimelineEditDialog");
    setWindowTitle(tr("Timeline editor"));

    setMinimumSize(320,200);

    settings->restoreGeometry(this);

    createWidgets_();
}

TimelineEditDialog::~TimelineEditDialog()
{
    settings->saveGeometry(this);
    delete tl_;
}

void TimelineEditDialog::createWidgets_()
{
    timer_ = new QTimer(this);
    timer_->setSingleShot(false);
    // timeout for emitting a changed signal
    timer_->setInterval(300);
    connect(timer_, SIGNAL(timeout()), this, SIGNAL(timelineChanged()));

    auto lv = new QVBoxLayout(this);

        editor_ = new Timeline1DRulerView(tl_, this);
        lv->addWidget(editor_);

        connect(editor_, SIGNAL(Timeline1DRulerView::timelineChanged),
                timer_, SLOT(start()));
}


void TimelineEditDialog::setTimeline(const MATH::Timeline1D & tl)
{
    if (!tl_)
        tl_ = new MATH::Timeline1D();
    *tl_ = tl;

    editor_->setTimeline(tl_);
}





} // namespace GUI
} // namespace MO
