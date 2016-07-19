/** @file resolutiondialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.10.2014</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "ResolutionDialog.h"
#include "io/Settings.h"
#include "widget/SpinBox.h"
#include "tool/CommonResolutions.h"

namespace MO {
namespace GUI {

ResolutionDialog::ResolutionDialog(const QSize& def, QWidget *parent)
    : QDialog   (parent),
      default_  (def),
      res_      (def)
{
    setObjectName("_ResolutionDialog");
    setWindowTitle(tr("Choose resolution"));

    setMinimumSize(320,200);

    settings()->restoreGeometry(this);

    createWidgets_();

    updateSpins_();
}

ResolutionDialog::~ResolutionDialog()
{
    settings()->storeGeometry(this);
}


void ResolutionDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // preset box

        auto cb = new QComboBox(this);
        lv->addWidget(cb);

        // add presets
        cb->addItem(tr("Custom..."));
        int k=0;
        for (auto & r : CommonResolutions::resolutions)
            cb->addItem(r.descriptiveName(), k++);

        // focus on current preset, if so
        if (auto r = CommonResolutions::findResolution(res_))
            cb->setCurrentText(r->descriptiveName());

        connect(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int idx)
        {
            idx--;
            if (idx >= 0 && idx < CommonResolutions::resolutions.size())
                setResolution(CommonResolutions::resolutions[idx].size());
            else
                setResolution(default_);
        });

        // width and height

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            s_width_ = new SpinBox(this);
            lh->addWidget(s_width_);
            s_width_->setRange(4, 4096*4);
            s_width_->setSuffix(tr(" px"));
            connect(s_width_, &SpinBox::valueChanged, [=](int v)
            {
                res_.setWidth(v);
            });

            auto l = new QLabel(tr("x"), this);
            l->setAlignment(Qt::AlignCenter);
            lh->addWidget(l);

            s_height_ = new SpinBox(this);
            lh->addWidget(s_height_);
            s_height_->setRange(4, 4096*4);
            s_height_->setSuffix(tr(" px"));
            connect(s_height_, &SpinBox::valueChanged, [=](int v)
            {
                res_.setHeight(v);
            });

        // flip button

        auto but = new QPushButton(tr("flip"), this);
        lv->addWidget(but);
        connect(but, &QPushButton::clicked, [=]()
        {
            auto w = res_.width();
            res_.setWidth(res_.height());
            res_.setHeight(w);
            updateSpins_();
        });

        lv->addStretch(1);

        // Ok/Cancel buttons

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            but = new QPushButton(tr("Ok"), this);
            lh->addWidget(but);
            but->setDefault(true);
            connect(but, SIGNAL(clicked()), this, SLOT(accept()));

            but = new QPushButton(tr("Cancel"), this);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SLOT(reject()));
}

void ResolutionDialog::setResolution(const QSize &size)
{
    res_ = size;

    updateSpins_();
}

void ResolutionDialog::updateSpins_()
{
    // update widgets
    s_width_->setValue(res_.width());
    s_height_->setValue(res_.height());
}

} // namespace GUI
} // namespace MO
