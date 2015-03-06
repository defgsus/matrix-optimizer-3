/** @file presetswidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QPushButton>

#include "presetswidget.h"
#include "gui/util/frontpreset.h"

namespace MO {
namespace GUI {


struct PresetsWidget::Private
{
    Private(PresetsWidget * widget)
        : widget        (widget)
        , ignoreCombo   (false)
    { }

    void createWidgets();
    void updateCombo();
    void setComboIndex(int);

    PresetsWidget * widget;
    FrontPresets * presets;
    QComboBox * combo;
    bool ignoreCombo;
};



PresetsWidget::PresetsWidget(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();
}

PresetsWidget::~PresetsWidget()
{
    if (p_->presets)
        p_->presets->releaseRef();
    delete p_;
}

void PresetsWidget::Private::createWidgets()
{
    auto lh = new QHBoxLayout(widget);
    lh->setMargin(0);

        combo = new QComboBox(widget);
        lh->addWidget(combo, 1);
        connect(combo, SIGNAL(currentIndexChanged(int)),
                widget, SLOT(onComboChanged_()));

        auto but = new QPushButton(tr("load"), widget);
        but->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        lh->addWidget(but);

        but = new QPushButton(tr("overwrite"), widget);
        but->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        lh->addWidget(but);
}

void PresetsWidget::Private::updateCombo()
{
    combo->clear();

    auto list = presets->presetsIds();
    for (auto p : list)
    {
        // add name/id
        combo->addItem(p.first->name(), p.second);
    }

    setComboIndex(0);
}

void PresetsWidget::Private::setComboIndex(int i)
{
    if (i == -1 || i < combo->count())
    {
        ignoreCombo = true;
        combo->setCurrentIndex(i);
        ignoreCombo = false;
    }
}

FrontPresets * PresetsWidget::presets() const
{
    return p_->presets;
}

void PresetsWidget::setPresets(FrontPresets * p)
{
    if (p_->presets)
        p_->presets->releaseRef();

    p_->presets = p;
    p_->presets->addRef();

    p_->updateCombo();
}

void PresetsWidget::onComboChanged_()
{
    if (p_->ignoreCombo)
        return;

    int i = p_->combo->currentIndex();
    if (i < 0 || i >= p_->combo->count())
        return;

    emit presetSelected(p_->combo->itemData(i).toString());
}

} // namespace GUI
} // namespace MO
