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
#include <QToolButton>
#include <QPlainTextEdit>
#include <QTimer>
#include <QMessageBox>

#include "midisettingsdialog.h"
#include "audio/mididevices.h"
#include "audio/mididevice.h"
#include "io/error.h"
#include "io/applicationtime.h"

namespace MO {
namespace GUI {


MidiSettingsDialog::MidiSettingsDialog(QWidget *parent)
    : QDialog   (parent),
      curId_    (-1),
      curDevice_(0),
      devices_  (0),
      timer_    (new QTimer(this))
{
    setObjectName("_MidiSettingsDialig");
    setWindowTitle(tr("Midi settings"));
    setMinimumSize(640,480);

    createWidgets_();
    checkDevices_();
    updateWidgets_();

    timer_->setSingleShot(false);
    timer_->setInterval(1000 / 30);
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer_()));
}

MidiSettingsDialog::~MidiSettingsDialog()
{
    delete devices_;
    delete curDevice_;
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
            connect(comboApi_, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onApiChoosen_()));


        lh = new QHBoxLayout();
        lv->addLayout(lh);

            label = new QLabel(tr("input device"), this);
            lh->addWidget(label);

            comboDevice_ = new QComboBox(this);
            lh->addWidget(comboDevice_);
            connect(comboDevice_, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onDeviceChoosen_()));

        // test area

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            butTest_ = new QToolButton(this);
            lh->addWidget(butTest_);
            butTest_->setText(tr("test input"));
            butTest_->setCheckable(true);
            connect(butTest_, SIGNAL(toggled(bool)), this, SLOT(onTest_(bool)));

            textBuffer_ = new QPlainTextEdit(this);
            lh->addWidget(textBuffer_);

    lv->addStretch(2);
}

void MidiSettingsDialog::checkDevices_()
{
    comboApi_->clear();
    comboApi_->addItem(tr("None"), -1);
    comboDevice_->clear();
    comboDevice_->addItem(tr("None"), -1);

    if (!devices_)
        devices_ = new AUDIO::MidiDevices();

    if (!devices_->checkDevices())
        return;

    int k=1;
    for (auto &n : devices_->apiNames())
    {
        comboApi_->addItem(n, n);
        if (curApi_ == n)
            comboApi_->setCurrentIndex(k);
        ++k;
    }

    updateDeviceBox_();
}

void MidiSettingsDialog::updateDeviceBox_()
{
    comboDevice_->clear();
    comboDevice_->addItem(tr("None"), -1);

    int k = 1;
    for (const AUDIO::MidiDevices::DeviceInfo& i : devices_->deviceInfos())
    {
        if (!i.output && i.apiName == curApi_)
            comboDevice_->addItem(i.name, i.id);
        if (curId_ == i.id)
            comboDevice_->setCurrentIndex(k);
        ++k;
    }
}

void MidiSettingsDialog::updateWidgets_()
{
    butTest_->setEnabled(curId_ >= 0);
}

void MidiSettingsDialog::onApiChoosen_()
{
    if (!devices_)
        return;

    const int idx = comboApi_->currentIndex();
    if (idx < 0 || idx > comboApi_->count())
        return;

    curApi_ = comboApi_->itemData(idx).toString();

    updateDeviceBox_();
}

void MidiSettingsDialog::onDeviceChoosen_()
{
    const int idx = comboDevice_->currentIndex();
    if (idx < 0 || idx > comboDevice_->count())
        return;

    curId_ = comboDevice_->itemData(idx).toInt();

    updateWidgets_();
}

void MidiSettingsDialog::onTest_(bool go)
{
    if (go)
    {
        curDevice_ = new AUDIO::MidiDevice();
        try
        {
            curDevice_->openInput(curId_);
            timer_->start();
            return;
        }
        catch (Exception& e)
        {
            QMessageBox::critical(this, tr("midi"),
                                  tr("Could not open the midi input device\n%1")
                                  .arg(e.what()));
            butTest_->setChecked(false);
        }
    }

    delete curDevice_;
    curDevice_ = 0;
}


void MidiSettingsDialog::onTimer_()
{
    if (!curDevice_)
        return;

    while (curDevice_->isInputEvent())
    {
        AUDIO::MidiEvent e = curDevice_->read();
        textBuffer_->appendPlainText(
                    "[" + applicationTimeString() + "] " + e.toString());
    }
}

} // namespace GUI
} // namespace MO
