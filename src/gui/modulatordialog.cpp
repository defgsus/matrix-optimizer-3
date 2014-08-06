/** @file modulatordialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include <QComboBox>
#include <QLayout>

#include "modulatordialog.h"
#include "object/param/modulator.h"
#include "widget/modulatorwidget.h"

Q_DECLARE_METATYPE(MO::Modulator*)

namespace MO {
namespace GUI {


ModulatorDialog::ModulatorDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("edit modulations"));

    auto lv = new QVBoxLayout(this);
    lv->setMargin(2);

        combo_ = new QComboBox(this);
        lv->addWidget(combo_);
        connect(combo_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(comboChanged_()));

        modWidget_ = new ModulatorWidget(this);
        lv->addWidget(modWidget_);

}


void ModulatorDialog::setModulators(const QList<Modulator *> mods, Modulator *select)
{
    combo_->clear();

    int idx = -1;
    for (auto m : mods)
    {
        if (m == select)
            idx = combo_->count();
        combo_->addItem(m->name(), QVariant::fromValue(m));
    }

    if (idx >= 0)
        combo_->setCurrentIndex(idx);
    else
        modWidget_->setModulator(select);
}

void ModulatorDialog::comboChanged_()
{
    const int idx = combo_->currentIndex();
    if (idx < 0)
        return;

    Modulator * m = combo_->itemData(idx).value<MO::Modulator*>();

    if (m)
        modWidget_->setModulator(m);
}

} // namespace GUI
} // namespace MO
