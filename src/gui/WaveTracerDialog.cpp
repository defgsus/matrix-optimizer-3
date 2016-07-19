/** @file wavetracerdialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.04.2015</p>
*/

#include <QLayout>

#include "WaveTracerDialog.h"
#include "widget/WaveTracerWidget.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {


WaveTracerDialog::WaveTracerDialog(QWidget *parent)
    : QDialog       (parent)
{
    setObjectName("_WaveTracerDialog");
    setWindowTitle(tr("IR Wave Tracer"));

    setMinimumSize(800, 600);

    auto lv = new QVBoxLayout(this);
    lv->setMargin(0);

    p_widget_ = new WaveTracerWidget(this);
    lv->addWidget(p_widget_);

    settings()->restoreGeometry(this);
}

WaveTracerDialog::~WaveTracerDialog()
{
    settings()->storeGeometry(this);
}

} // namespace GUI
} // namespace MO
