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

namespace MO {
namespace GUI {

AudioFilterDialog::AudioFilterDialog(QWidget *parent) :
    QDialog     (parent),
    filter_     (new AUDIO::MultiFilter()),
    display_    (new FilterResponseWidget(this))
{
    setObjectName("AudioFilterDialog");
    setMinimumSize(320,240);
    resize(800,600);

    settings->restoreGeometry(this);

    restoreFilter_();

    createWidgets_();
    updateVisibility_();

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

            combo->setCurrentText(filter_->typeName());

            // frequency

            label = new QLabel(tr("frequency"), this);
            lv->addWidget(label);

            auto dspin = dspinFreq_ = new DoubleSpinBox(this);
            lv->addWidget(dspin);
            dspin->setMinimum(0.0001);
            dspin->setMaximum(display_->sampleRate()/2);
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

            label = labelReso_ = new QLabel(tr("resonance"), this);
            lv->addWidget(label);

            dspin = dspinReso_ = new DoubleSpinBox(this);
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

            label = labelOrder_ = new QLabel(tr("filter order"), this);
            lv->addWidget(label);

            auto spin = spinOrder_ = new SpinBox(this);
            lv->addWidget(spin);
            spin->setMinimum(1);
            spin->setMaximum(30);
            spin->setValue(filter_->order());
            connect(spin, &SpinBox::valueChanged, [=]()
            {
                filter_->setOrder(spin->value());
                display_->setFilter(*filter_);
            });

            // combo change
            connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]()
            {
                int idx = combo->currentIndex();
                if (idx < 0 || idx >= AUDIO::MultiFilter::filterTypeEnums.size())
                    return;

                filter_->setType((AUDIO::MultiFilter::FilterType)
                                 combo->itemData(idx).toInt());

                updateVisibility_();

                display_->setFilter(*filter_);
            });

            // frequency choose from display
            connect(display_, &FilterResponseWidget::frequencyChanged, [=](F32 f)
            {
                dspinFreq_->setValue(f, true);
            });

            lv->addStretch(1);

            // resolution

            label = new QLabel(tr("display resolution"), this);
            lv->addWidget(label);

            spin = new SpinBox(this);
            lv->addWidget(spin);
            spin->setMinimum(8);
            spin->setMaximum(1024);
            spin->setValue(display_->numBands());
            spin->setSuffix(tr(" bands"));
            connect(spin, &SpinBox::valueChanged, [=](int n)
            {
                display_->setNumBands(n);
            });

            label = new QLabel(tr("buffer length"), this);
            lv->addWidget(label);

            spin = new SpinBox(this);
            lv->addWidget(spin);
            spin->setMinimum(8);
            spin->setMaximum(8192);
            spin->setValue(display_->bufferSize());
            connect(spin, &SpinBox::valueChanged, [=](int n)
            {
                display_->setBufferSize(n);
            });
}

void AudioFilterDialog::saveFilter_()
{
    settings->setValue("AudioFilterDialog/type", filter_->typeId());
    settings->setValue("AudioFilterDialog/frequency", (Double)filter_->frequency());
    settings->setValue("AudioFilterDialog/resonance", (Double)filter_->resonance());
    settings->setValue("AudioFilterDialog/order", filter_->order());
    settings->setValue("AudioFilterDialog/resolution", display_->numBands());
    settings->setValue("AudioFilterDialog/bufferSize", display_->bufferSize());
}

void AudioFilterDialog::restoreFilter_()
{
    filter_->setType(settings->value("AudioFilterDialog/type", filter_->typeId()).toString());
    filter_->setFrequency(settings->value("AudioFilterDialog/frequency", (Double)filter_->frequency()).toDouble());
    filter_->setResonance(settings->value("AudioFilterDialog/resonance", (Double)filter_->resonance()).toDouble());
    filter_->setOrder(settings->value("AudioFilterDialog/order", filter_->order()).toInt());
    display_->setNumBands(settings->value("AudioFilterDialog/resolution", display_->numBands()).toInt(), false);
    display_->setBufferSize(settings->value("AudioFilterDialog/bufferSize", display_->bufferSize()).toInt(), false);
}

void AudioFilterDialog::updateVisibility_()
{
    const bool isorder = AUDIO::MultiFilter::supportsOrder(filter_->type());
    labelOrder_->setVisible(isorder);
    spinOrder_->setVisible(isorder);
    const bool isreso = AUDIO::MultiFilter::supportsResonance(filter_->type());
    labelReso_->setVisible(isreso);
    dspinReso_->setVisible(isreso);
}

} // namespace GUI
} // namespace MO
