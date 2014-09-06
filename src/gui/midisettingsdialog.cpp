/** @file midisettingsdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

#include "midisettingsdialog.h"
#include "audio/mididevices.h"

namespace MO {
namespace GUI {


MidiSettingsDialog::MidiSettingsDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_MidiSettingsDialig");
    setWindowTitle(tr("Midi settings"));
    setMinimumSize(640,480);

    createWidgets_();
    updateDeviceBox_();
}


void MidiSettingsDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // --- apis ---

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto label = new QLabel(tr("api"), this);
            lh->addWidget(label);

            comboApi_ = new QComboBox(this);
            lh->addWidget(comboApi_);

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            label = new QLabel(tr("device"), this);
            lh->addWidget(label);

            comboDevice_ = new QComboBox(this);
            lh->addWidget(comboDevice_);

}

void MidiSettingsDialog::updateDeviceBox_()
{
    comboApi_->clear();
    comboApi_->addItem(tr("None"), -1);
    comboDevice_->clear();
    comboDevice_->addItem(tr("None"), -1);

    AUDIO::MidiDevices devs;
    if (!devs.checkDevices())
        return;

    for (auto &n : devs.apiNames())
        comboApi_->addItem(n, n);

    for (const AUDIO::MidiDevices::DeviceInfo& i : devs.deviceInfos())
    {
        QString name = "[";
        if (i.input)
            name += "I";
        if (i.output)
        {
            if (i.input)
                name += "/";
            name += "O";
        }
        name += "] " + i.name;

        comboDevice_->addItem(name, i.id);
    }
}



} // namespace GUI
} // namespace MO
