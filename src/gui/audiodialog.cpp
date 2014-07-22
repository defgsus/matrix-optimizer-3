/** @file audiosettings.cpp

    @brief Audio settings dialog

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QMessageBox>
#include <QToolButton>
#include <QSpinBox>

#include "audiodialog.h"
#include "audio/audiodevices.h"
#include "audio/audiodevice.h"
#include "io/error.h"
#include "math/constants.h"

namespace MO {
namespace GUI {


AudioDialog::AudioDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog   (parent, f),
      device_   (0),
      freq_     (437),
      vol_      (20),
      phase_    (0.f)
{
    setObjectName("_AudioDialog");
    setWindowTitle(tr("audio settings"));
    setModal(true);

    setMinimumSize(400,300);

    devices_ = new AUDIO::AudioDevices();

    auto l0 = new QVBoxLayout(this);

        apiBox_ = new QComboBox(this);
        l0->addWidget(apiBox_);
        connect(apiBox_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(fillDeviceBox_()));

        deviceBox_ = new QComboBox(this);
        l0->addWidget(deviceBox_);
        connect(deviceBox_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(deviceSelected_()));

        // --- test tone ---

        auto lh = new QHBoxLayout();
        l0->addLayout(lh);

            testButt_ = new QToolButton(this);
            lh->addWidget(testButt_);
            testButt_->setIcon(QIcon(":/icon/obj_soundsource.png"));
            testButt_->setText(tr("Test tone"));
            testButt_->setToolTip(tr("Test tone"));
            testButt_->setCheckable(true);
            testButt_->setEnabled(false);
            connect(testButt_, SIGNAL(toggled(bool)),
                    this, SLOT(toggleTesttone_()));

            auto freqSpin = new QSpinBox(this);
            lh->addWidget(freqSpin);
            freqSpin->setRange(1, 44000);
            freqSpin->setValue(freq_);
            freqSpin->setSuffix(tr(" hz"));
            connect(freqSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]()
            {
                freq_ = freqSpin->value();
            });

            auto volSpin = new QSpinBox(this);
            lh->addWidget(volSpin);
            volSpin->setRange(1,100);
            volSpin->setValue(vol_);
            connect(freqSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]()
            {
                vol_ = volSpin->value();
            });

        l0->addStretch(1);

    checkDevices_();
}

AudioDialog::~AudioDialog()
{
    delete devices_;
    if (device_)
        delete device_;
}



void AudioDialog::checkDevices_()
{
    apiBox_->clear();
    apiBox_->addItem(tr("None"), QVariant(-1));
    deviceBox_->clear();
    deviceBox_->addItem(tr("None"), QVariant(-1));

    try
    {
        devices_->checkDevices();
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to initialize audio devices\n%1").arg(e.what())
                             );
        return;
    }

    // fill api box
    for (uint i=0; i<devices_->numApis(); ++i)
    {
        auto inf = devices_->getApiInfo(i);
        apiBox_->addItem(inf->name, QVariant(i));
    }

    fillDeviceBox_();
}


void AudioDialog::fillDeviceBox_()
{
    deviceBox_->clear();
    deviceBox_->addItem(tr("None"), QVariant(-1));

    if (apiBox_->currentIndex() <= 0)
        return;

    uint apiIndex = apiBox_->currentIndex() - 1;

    for (uint i=0; i<devices_->numDevices(); ++i)
    {
        auto inf = devices_->getDeviceInfo(i);
        if (inf->apiIndex != apiIndex)
            continue;

        deviceBox_->addItem(QString("%1:%2 %3")
                            .arg(inf->numInputChannels)
                            .arg(inf->numOutputChannels)
                            .arg(inf->name), QVariant(i));
    }
}


void AudioDialog::deviceSelected_()
{
    testButt_->setEnabled( deviceBox_->currentIndex() > 0);
}

int AudioDialog::selectedDeviceIndex() const
{
    int idx = deviceBox_->currentIndex();
    if (idx < 1)
        return -1;

    return deviceBox_->itemData(idx).toInt();
}

void AudioDialog::toggleTesttone_()
{
    if (testButt_->isChecked())
    {
        startTone();
    }
    else
        stopTone();
}

void AudioDialog::startTone()
{
    if (!device_)
        device_ = new AUDIO::AudioDevice();
    else
    {
        device_->stop();
        device_->close();
    }

    int idx = selectedDeviceIndex();
    if (idx < 0)
        return;

    auto inf = devices_->getDeviceInfo(idx);

    AUDIO::Configuration conf;
    conf.setNumChannelsIn(0);
    conf.setNumChannelsOut(inf->numOutputChannels);
    conf.setBufferSize(inf->defaultBufferLength);
    conf.setSampleRate(inf->defaultSampleRate);


    device_->setCallback([=](const F32*, F32* out)
    {
        for (uint i=0; i<conf.bufferSize(); ++i)
        {
            F32 sam = 0.01f * vol_ * sin(phase_ * TWO_PI * freq_);

            for (uint j=0; j<conf.numChannelsOut(); ++j)
                *out++ = sam;

            phase_ += 1.f / conf.sampleRate();
            if (phase_ > 1.f)
                phase_ -= 2.f;
            else if (phase_ < -1.f)
                phase_ += 2.f;
        }
    });

    try
    {
        device_->init(idx, conf);
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to init audio device '%1'\n%2")
                             .arg(inf->name)
                             .arg(e.what())
                             );
        return;
    }


    try
    {
        device_->start();
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to start audio device '%1'\n%2")
                             .arg(inf->name)
                             .arg(e.what())
                             );
        return;
    }
}


void AudioDialog::stopTone()
{
    if (device_)
    {
        device_->stop();
        device_->close();
    }
}



} // namespace GUI
} // namespace MO
