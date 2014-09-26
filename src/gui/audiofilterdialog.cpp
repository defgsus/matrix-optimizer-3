/** @file audiofilterdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QLabel>

#include "audiofilterdialog.h"
#include "widget/filterresponsewidget.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "audio/tool/multifilter.h"
#include "io/settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

AudioFilterDialog::AudioFilterDialog(QWidget *parent) :
    QDialog     (parent),
    filter_     (new AUDIO::MultiFilter())
{
    setObjectName("AudioFilterDialog");
    setMinimumSize(320,240);
    resize(800,600);

    settings->restoreGeometry(this);

    restoreFilter_();

    createWidgets_();

    display_->setFilter(*filter_);
}

AudioFilterDialog::~AudioFilterDialog()
{
    saveFilter_();
    settings->saveGeometry(this);

    delete filter_;
}

void AudioFilterDialog::createWidgets_()
{
    auto lh = new QHBoxLayout(this);

        display_ = new FilterResponseWidget(this);
        display_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lh->addWidget(display_);

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            // filter type

            auto label = new QLabel(tr("filter type"), this);
            lv->addWidget(label);

            auto combo = new QComboBox(this);
            lv->addWidget(combo);

            for (int i=0; i<AUDIO::MultiFilter::filterTypeNames.size(); ++i)
                combo->addItem(
                            AUDIO::MultiFilter::filterTypeNames[i],
                            (int)AUDIO::MultiFilter::filterTypeEnums[i]);
            MO_DEBUG("SetCurrent " << filter_->typeName());
            combo->setCurrentText(filter_->typeName());

            // frequency

            label = new QLabel(tr("frequency"), this);
            lv->addWidget(label);

            auto dspin = new DoubleSpinBox(this);
            lv->addWidget(dspin);
            dspin->setMinimum(0.0001);
            dspin->setMaximum(display_->sampleRate());
            dspin->setDecimals(6);
            dspin->setSuffix(tr(" hz"));
            dspin->setValue(filter_->frequency());
            dspin->setSingleStep(200);
            connect(dspin, &DoubleSpinBox::valueChanged, [=]()
            {
                filter_->setFrequency(dspin->value());
                display_->setFilter(*filter_);
            });

            // resonance

            label = new QLabel(tr("resonance"), this);
            lv->addWidget(label);

            dspin = new DoubleSpinBox(this);
            lv->addWidget(dspin);
            dspin->setMinimum(0.0);
            dspin->setMaximum(1.0);
            dspin->setDecimals(6);
            dspin->setValue(filter_->resonance());
            dspin->setSingleStep(0.02);
            connect(dspin, &DoubleSpinBox::valueChanged, [=]()
            {
                filter_->setResonance(dspin->value());
                display_->setFilter(*filter_);
            });

            // order

            label = new QLabel(tr("filter order"), this);
            lv->addWidget(label);

            auto spin = new SpinBox(this);
            lv->addWidget(spin);
            spin->setMinimum(1);
            spin->setMaximum(30);
            spin->setValue(filter_->order());
            connect(spin, &SpinBox::valueChanged, [=]()
            {
                filter_->setOrder(spin->value());
                display_->setFilter(*filter_);
            });


            connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]()
            {
                int idx = combo->currentIndex();
                if (idx < 0 || idx >= AUDIO::MultiFilter::filterTypeEnums.size())
                    return;

                filter_->setType((AUDIO::MultiFilter::FilterType)
                                 combo->itemData(idx).toInt());

                bool isorder = AUDIO::MultiFilter::supportsOrder(filter_->type());
                label->setVisible(isorder);
                spin->setVisible(isorder);

                display_->setFilter(*filter_);
            });

            lv->addStretch(1);
}

void AudioFilterDialog::saveFilter_()
{
    settings->setValue("AudioFilterDialog/type", filter_->typeId());
    settings->setValue("AudioFilterDialog/frequency", (Double)filter_->frequency());
    settings->setValue("AudioFilterDialog/resonance", (Double)filter_->resonance());
    settings->setValue("AudioFilterDialog/order", filter_->order());
}

void AudioFilterDialog::restoreFilter_()
{
    filter_->setType(settings->value("AudioFilterDialog/type", filter_->typeId()).toString());
    filter_->setFrequency(settings->value("AudioFilterDialog/frequency", (Double)filter_->frequency()).toDouble());
    filter_->setResonance(settings->value("AudioFilterDialog/resonance", (Double)filter_->resonance()).toDouble());
    filter_->setOrder(settings->value("AudioFilterDialog/order", filter_->order()).toInt());
}


} // namespace GUI
} // namespace MO
