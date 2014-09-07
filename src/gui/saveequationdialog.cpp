/** @file saveequationdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/7/2014</p>
*/

#include <QLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QStringListModel>
#include <QCompleter>

#include "saveequationdialog.h"
#include "io/equationpresets.h"
#include "io/equationpreset.h"

namespace MO {
namespace GUI {


SaveEquationDialog::SaveEquationDialog(const QString& pname, QWidget *parent)
    : QDialog       (parent),
      presets_      (new IO::EquationPresets()),
      curGroup_     (pname)
{
    setObjectName("_SaveEquationDialog");
    setWindowTitle(tr("Save equation"));

    setMinimumSize(320,240);

    createWidgets_();
    onGroupSelect_();
}

SaveEquationDialog::SaveEquationDialog(QWidget *parent)
    : SaveEquationDialog("", parent)
{

}


SaveEquationDialog::~SaveEquationDialog()
{
    delete presets_;
}

void SaveEquationDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // --- presets group ---

        comboGroup_ = new QComboBox(this);
        lv->addWidget(comboGroup_);

        comboGroup_->addItem("-new group-", "");

        for (int i=0; i<presets_->count(); ++i)
        {
            comboGroup_->addItem(presets_->name(i), QVariant(presets_->name(i)));
        }
        if (curGroup_.isEmpty())
            comboGroup_->setCurrentIndex(0);
        else
            comboGroup_->setCurrentText(curGroup_);

        connect(comboGroup_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onGroupSelect_()));

        // --- preset name ---

        auto l = new QLabel(tr("Enter name of the new preset"), this);
        lv->addWidget(l);

        edit_ = new QLineEdit(this);
        lv->addWidget(edit_);


        // ---- ok/cancel ----

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("Ok"), this);
            lh->addWidget(but);

            but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
}

void SaveEquationDialog::onGroupSelect_()
{
    const int idx = comboGroup_->currentIndex();
    if (idx < 0 || idx >= comboGroup_->count())
        return;

    curGroup_ = comboGroup_->itemData(idx).toString();

    updateCompleter_();
}

void SaveEquationDialog::updateCompleter_()
{
    const IO::EquationPreset * p = 0;
    for (int i = 0; i<presets_->count(); ++i)
        if (curGroup_ == presets_->name(i))
            p = presets_->preset(i);

    QStringList list;
    if (p)
    {
        for (auto i = 0; i<p->count(); ++i)
            list << p->equationName(i);
    }

    auto model = new QStringListModel(list, edit_);

    auto c = new QCompleter(model, edit_);

    edit_->setCompleter(c);
}

} // namespace GUI
} // namespace MO
