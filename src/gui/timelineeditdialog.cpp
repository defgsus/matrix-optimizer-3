/** @file timelineeditdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#include <QLayout>

#include "timelineeditdialog.h"
#include "math/timeline1d.h"
#include "timeline1dview.h"

namespace MO {
namespace GUI {

TimelineEditDialog::TimelineEditDialog(QWidget *parent)
    : QDialog       (parent),
      tl_           (0)
{
    setObjectName("_TimelineEditDialog");
    setWindowTitle(tr("Timeline editor"));

    createWidgets_();
}

void TimelineEditDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        editor_ = new Timeline1DView(tl_, this);
        lv->addWidget(editor_);
}


void TimelineEditDialog::setTimeline(MATH::Timeline1D * tl)
{
    tl_ = tl;

    editor_->setTimeline(tl_);
}





} // namespace GUI
} // namespace MO
