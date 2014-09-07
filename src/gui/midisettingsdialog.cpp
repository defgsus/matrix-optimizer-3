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
#include "io/settings.h"
#include "io/log.h"

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

    timer_->setSingleShot(false);
    timer_->setInterval(1000 / 30);
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer_()));

    createWidgets_();

    loadSettings_();
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

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto butOk = new QPushButton(tr("Ok"), this);
            lh->addWidget(butOk);
            butOk->setDefault(true);
            connect(butOk, SIGNAL(clicked()), this, SLOT(onOk_()));

            auto butCancel = new QPushButton(tr("Cancel"), this);
            lh->addWidget(butCancel);
            connect(butCancel, SIGNAL(clicked()), this, SLOT(reject()));

}

void MidiSettingsDialog::checkDevices_()
{
    const QString capi(curApiName_), cdev(curDeviceName_);
    const int cid(curId_);

    comboApi_->clear();
    comboApi_->addItem(tr("None"), -1);
    comboDevice_->clear();
    comboDevice_->addItem(tr("None"), -1);

    curApiName_ = capi;
    curDeviceName_ = cdev;
    curId_ = cid;

    if (!devices_)
        devices_ = new AUDIO::MidiDevices();

    if (!devices_->checkDevices())
        return;

    int k=1;
    for (auto &n : devices_->apiNames())
    {
        comboApi_->addItem(n, n);
        if (curApiName_ == n)
            comboApi_->setCurrentIndex(k);
        ++k;
    }

    updateDeviceBox_();
}

void MidiSettingsDialog::updateDeviceBox_()
{
    const QString cdev(curDeviceName_);
    const int cid(curId_);

    comboDevice_->clear();
    comboDevice_->addItem(tr("None"), -1);

    curDeviceName_ = cdev;
    curId_ = cid;

    int k = 1;
    for (const AUDIO::MidiDevices::DeviceInfo& i : devices_->deviceInfos())
    {
        if (i.output || i.apiName != curApiName_)
            continue;

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

    curApiName_ = comboApi_->itemData(idx).toString();

    updateDeviceBox_();
}

void MidiSettingsDialog::onDeviceChoosen_()
{
    const int idx = comboDevice_->currentIndex();
    if (idx < 0 || idx > comboDevice_->count())
        return;

    curId_ = comboDevice_->itemData(idx).toInt();
    curDeviceName_ = devices_->nameForId(curId_);

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

void MidiSettingsDialog::saveSettings_()
{
    MO_DEBUG_MIDI("saving midi settings " << curApiName_ << "/" << curDeviceName_);

    if (curId_ < 0 || curDeviceName_.isEmpty())
    {
        settings->setValue("MidiIn/api", "");
        settings->setValue("MidiIn/device", "");
    }
    else
    {
        settings->setValue("MidiIn/api", curApiName_);
        settings->setValue("MidiIn/device", curDeviceName_);
    }
}

void MidiSettingsDialog::loadSettings_()
{
    curApiName_ = settings->getValue("MidiIn/api").toString();
    curDeviceName_ = settings->getValue("MidiIn/device").toString();

    if (!devices_)
        devices_ = new AUDIO::MidiDevices;
    devices_->checkDevices();

    curId_ = devices_->idForName(curDeviceName_, false);

    MO_DEBUG("loaded midi settings " << curApiName_ << "/"
             << curDeviceName_ << " today's id=" << curId_);

    checkDevices_();
}

void MidiSettingsDialog::onOk_()
{
    saveSettings_();
    accept();
}

} // namespace GUI
} // namespace MO
