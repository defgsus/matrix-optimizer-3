/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/29/2016</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

#include "InsertTimeDialog.h"
#include "widget/DoubleSpinBox.h"
#include "object/Scene.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {

InsertTimeDialog::InsertTimeDialog(QWidget *parent)
    : QDialog       (parent)
    , where_        (0.)
    , howMuch_      (10.)
{
    setObjectName("_TimeDialog");
    setWindowTitle(tr("Insert time"));
    setMinimumSize(320, 200);

    settings()->restoreGeometry(this);

    auto lv = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // where
            auto lv1 = new QVBoxLayout();
            lh->addLayout(lv1);

                lv1->addWidget(new QLabel(tr("insert at")));
                spinWhere_ = new DoubleSpinBox(this);
                spinWhere_->setDecimals(6);
                spinWhere_->setRange(0., 99999999.);
                connect(spinWhere_, &DoubleSpinBox::valueChanged, [=]()
                {
                    where_ = spinWhere_->value();
                });
                lv1->addWidget(spinWhere_);

                cbLocators_ = new QComboBox(this);
                cbLocators_->setVisible(false);
                connect(cbLocators_, static_cast<void(QComboBox::*)(int)>(
                            &QComboBox::currentIndexChanged), [=](int i)
                {
                    if (i >= 0 && i < cbLocators_->count())
                        setWhere(cbLocators_->itemData(i).toDouble());
                });
                lv1->addWidget(cbLocators_);

            // how much
            lv1 = new QVBoxLayout();
            lh->addLayout(lv1);

                lv1->addWidget(new QLabel(tr("length")));
                spinHowMuch_ = new DoubleSpinBox(this);
                spinHowMuch_->setDecimals(6);
                spinHowMuch_->setRange(0., 99999999.);
                spinHowMuch_->setValue(howMuch_);
                connect(spinHowMuch_, &DoubleSpinBox::valueChanged, [=]()
                {
                    howMuch_ = spinHowMuch_->value();
                });
                lv1->addWidget(spinHowMuch_);

                lv1->addStretch();

        lv->addStretch();

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("&Ok"), this);
            but->setDefault(true);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(accept()));
            lv->addWidget(but);

            but = new QPushButton(tr("&Cancel"), this);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(reject()));
            lv->addWidget(but);
}

InsertTimeDialog::~InsertTimeDialog()
{
    settings()->storeGeometry(this);
}

void InsertTimeDialog::setScene(Scene *scene)
{
    scene_ = scene;
    if (!scene_)
    {
        cbLocators_->setVisible(false);
        return;
    }
    cbLocators_->clear();
    for (auto i = scene_->locators().begin(); i != scene_->locators().end(); ++i)
    {
        if (cbLocators_->count() == 0 && i->time != 0.)
            cbLocators_->addItem(tr("beginning"), QVariant(0.));
        cbLocators_->addItem(tr("locator '%1': %2").arg(i->id).arg(i->time),
                             QVariant(i->time));
    }
    cbLocators_->setVisible(true);
}

void InsertTimeDialog::setWhere(Double where)
{
    where_ = where;
    spinWhere_->setValue(where);
}

void InsertTimeDialog::setHowMuch(Double howMuch)
{
    howMuch_ = howMuch;
    spinHowMuch_->setValue(howMuch);
}


} // namespace GUI
} // namespace MO
