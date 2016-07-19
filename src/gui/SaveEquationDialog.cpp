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
#include <QMessageBox>

#include "SaveEquationDialog.h"
#include "io/EquationPresets.h"
#include "io/EquationPreset.h"
#include "io/Files.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {


SaveEquationDialog::SaveEquationDialog(
                        const QString & equation, const QString& pname, QWidget *parent)
    : QDialog       (parent),
      presets_      (new IO::EquationPresets()),
      curGroup_     (pname),
      equation_     (equation)
{
    setObjectName("_SaveEquationDialog");
    setWindowTitle(tr("Save equation"));

    setMinimumSize(320,240);

    createWidgets_();
    onEditChanged_(); // update button visibility
    onGroupSelect_(); // update edit completer
    updateGroupCompleter_();
}

SaveEquationDialog::SaveEquationDialog(const QString& equation, QWidget *parent)
    : SaveEquationDialog(equation, "", parent)
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
        MO_DEBUG("curGroup " << curGroup_);
        if (curGroup_.isEmpty())
            comboGroup_->setCurrentIndex(0);
        else
            comboGroup_->setCurrentText(curGroup_);

        connect(comboGroup_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onGroupSelect_()));

        // --- preset name ---

        labelGroup_ = new QLabel(tr("Enter name of the new preset group"), this);
        lv->addWidget(labelGroup_);

        editGroup_ = new QLineEdit(this);
        lv->addWidget(editGroup_);
        connect(editGroup_, SIGNAL(textChanged(QString)),
                this, SLOT(onEditChanged_()));

        auto l = new QLabel(tr("Enter name of the new preset"), this);
        lv->addWidget(l);

        edit_ = new QLineEdit(this);
        lv->addWidget(edit_);
        connect(edit_, SIGNAL(textChanged(QString)),
                this, SLOT(onEditChanged_()));

        lv->addStretch(2);

        // ---- ok/cancel ----

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            butOk_ = new QPushButton(tr("Ok"), this);
            lh->addWidget(butOk_);
            connect(butOk_, SIGNAL(clicked()), this, SLOT(onOk_()));

            auto but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));

}

void SaveEquationDialog::onGroupSelect_()
{
    const int idx = comboGroup_->currentIndex();
    if (idx < 0 || idx >= comboGroup_->count())
        return;

    // will be empty for "new group"
    curGroup_ = comboGroup_->itemData(idx).toString();

    bool v = curGroup_.isEmpty();
    editGroup_->setVisible(v);
    labelGroup_->setVisible(v);

    updateCompleter_();
}

IO::EquationPreset * SaveEquationDialog::currentPreset_() const
{
    for (int i = 0; i<presets_->count(); ++i)
        if (curGroup_ == presets_->name(i))
            return presets_->preset(i);
    return 0;
}

void SaveEquationDialog::updateGroupCompleter_()
{
    QStringList list;
    for (auto i = 0; i<presets_->count(); ++i)
            list << presets_->name(i);

    auto model = new QStringListModel(list, editGroup_);

    auto c = new QCompleter(model, editGroup_);

    editGroup_->setCompleter(c);
}

void SaveEquationDialog::updateCompleter_()
{
    const IO::EquationPreset * p = currentPreset_();

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

void SaveEquationDialog::onEditChanged_()
{
    butOk_->setEnabled( !edit_->text().isEmpty()
        && (!curGroup_.isEmpty() || !editGroup_->text().isEmpty()));
}

void SaveEquationDialog::onOk_()
{
    QString equName = edit_->text();
    if (equName.isEmpty())
        return;

    // get preset

    IO::EquationPreset * preset = currentPreset_();

    // save to new group?
    if (preset == 0)
    {
        QString groupName = editGroup_->text();
        if (groupName.isEmpty())
            return;

        // querry preset file
        QString filename = IO::Files::getSaveFileName(IO::FT_EQUATION_PRESET, this, false, false);
        if (filename.isEmpty())
            return;

        IO::EquationPreset p;
        p.setName(groupName);
        p.setEquation(equName, equation_);
        try
        {
            p.save(filename);
        }
        catch (Exception& e)
        {
            QMessageBox::critical(this, tr("saving equation"),
                    tr("Sorry but saving the equation preset '%1' failed\n%2")
                                  .arg(groupName).arg(e.what()));
            return;
        }

        curGroup_ = groupName;
        equationName_ = equName;
        accept();
        return;
    }

    // check if present already
    if (preset->hasEquation(equName))
    {
        QMessageBox::Button res =
        QMessageBox::question(this, tr("confirm overwrite"),
            tr("Preset collection <b>%1</b> already has an equation called <b>%2</b>."
               "<br/>Do you want to overwrite it?").arg(curGroup_).arg(equName),
              QMessageBox::Yes | QMessageBox::No,
              QMessageBox::No);
        if (res == QMessageBox::No)
            return;
    }

    preset->setEquation(equName, equation_);
    try
    {
        preset->save();
    }
    catch (Exception & e)
    {
        QMessageBox::critical(this, tr("saving equation"),
                tr("Sorry but saving the equation preset '%1' failed\n%2")
                              .arg(curGroup_).arg(e.what()));
        return;
    }

    equationName_ = equName;
    accept();
}

} // namespace GUI
} // namespace MO
