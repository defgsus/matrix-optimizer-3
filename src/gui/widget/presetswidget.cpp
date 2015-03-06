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
    void updateButtons();
    void setComboIndex(int);
    void setComboIndex(const QString& id);

    // actions
    void newPreset();

    PresetsWidget * widget;
    FrontPresets * presets;
    QComboBox * combo;
    QPushButton * butLoad, * butSave, * butNew;
    bool ignoreCombo;
};



PresetsWidget::PresetsWidget(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();
    p_->updateButtons();
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

        butLoad = new QPushButton(tr("L"), widget);
        butLoad->setStatusTip(tr("Loads the currently selected preset"));
        butLoad->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        lh->addWidget(butLoad);
        connect(butLoad, &QPushButton::clicked, [=]()
        {
            QString id = widget->currentPresetId();
            if (!id.isEmpty())
                widget->presetLoadRequest(id);
        });

        butSave = new QPushButton(tr("S"), widget);
        butSave->setStatusTip(tr("Overwrites the currently selected preset"));
        butSave->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        lh->addWidget(butSave);
        connect(butSave, &QPushButton::clicked, [=]()
        {
            QString id = widget->currentPresetId();
            if (!id.isEmpty())
                widget->presetSaveRequest(id);
        });

        butNew = new QPushButton(tr("N"), widget);
        butNew->setStatusTip(tr("Creates a new preset"));
        butNew->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        lh->addWidget(butNew);
        connect(butNew, &QPushButton::clicked, [=](){ newPreset(); });

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

    //setComboIndex(0);
}

void PresetsWidget::Private::updateButtons()
{
    int i = combo->currentIndex();
    bool valid = (i >= 0 && i < combo->count());

    butLoad->setEnabled(valid);
    butSave->setEnabled(valid);
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

void PresetsWidget::Private::setComboIndex(const QString& id)
{
    for (int i=0; i<combo->count(); ++i)
    if (combo->itemData(i).toString() == id)
    {
        ignoreCombo = true;
        combo->setCurrentIndex(i);
        ignoreCombo = false;
        return;
    }
}

FrontPresets * PresetsWidget::presets() const
{
    return p_->presets;
}

QString PresetsWidget::currentPresetId() const
{
    int i = p_->combo->currentIndex();
    return (i >= 0 && i < p_->combo->count())
            ? p_->combo->itemData(i).toString()
            : QString();
}

void PresetsWidget::selectPreset(const QString &id)
{
    p_->setComboIndex(id);
}

void PresetsWidget::setPresets(FrontPresets * p)
{
    if (p_->presets)
        p_->presets->releaseRef();

    p_->presets = p;
    p_->presets->addRef();

    p_->updateCombo();
}

void PresetsWidget::updatePresets()
{
    p_->updateCombo();
    p_->updateButtons();
}

void PresetsWidget::onComboChanged_()
{
    p_->updateButtons();

    //emit presetSelected(p_->combo->itemData(i).toString());
}

void PresetsWidget::Private::newPreset()
{
    if (!presets)
        presets = new FrontPresets();

    const QString id = presets->uniqueId();
    presets->newPreset(id, id);

    updateCombo();
    setComboIndex(id);
}


} // namespace GUI
} // namespace MO
