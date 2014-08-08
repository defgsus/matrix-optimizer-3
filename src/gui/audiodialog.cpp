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
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QTimer>

#include "audiodialog.h"
#include "audio/audiodevices.h"
#include "audio/audiodevice.h"
#include "io/error.h"
#include "math/constants.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


AudioDialog::AudioDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog   (parent, f),
      device_   (0),
      freq_     (437),
      vol_      (20),
      realfreq_ (freq_),
      phase_    (0.f),
      doEnv_    (false)
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
                this, SLOT(apiSelected_()));

        deviceBox_ = new QComboBox(this);
        l0->addWidget(deviceBox_);
        connect(deviceBox_, SIGNAL(currentIndexChanged(int)),
                this, SLOT(deviceSelected_()));

        // buffersize
        auto lh = new QHBoxLayout();
        l0->addLayout(lh);

            auto label = new QLabel(tr("buffer size"), this);
            lh->addWidget(label);

            bufferSize_ = new QSpinBox(this);
            lh->addWidget(bufferSize_);
            bufferSize_->setRange(8, 4096*8);
            bufferSize_->setValue(settings->getValue("Audio/buffersize").toInt());

            auto but = new QToolButton(this);
            lh->addWidget(but);
            but->setText("D");
            but->setToolTip(tr("Set default value"));
            connect(but, SIGNAL(clicked()), this, SLOT(setDefaultBuffersize_()));

        // samplerate
        lh = new QHBoxLayout();
        l0->addLayout(lh);

            label = new QLabel(tr("samplerate"), this);
            lh->addWidget(label);

            sampleRate_ = new QSpinBox(this);
            lh->addWidget(sampleRate_);
            sampleRate_->setRange(8, 256000);
            sampleRate_->setValue(settings->getValue("Audio/samplerate").toInt());

            but = new QToolButton(this);
            lh->addWidget(but);
            but->setText("D");
            but->setToolTip(tr("Set default value"));
            connect(but, SIGNAL(clicked()), this, SLOT(setDefaultSamplerate_()));

        // channels in
        lh = new QHBoxLayout();
        l0->addLayout(lh);

            label = new QLabel(tr("input channels"), this);
            lh->addWidget(label);

            numInputs_ = new QSpinBox(this);
            lh->addWidget(numInputs_);
            numInputs_->setRange(0, 256);
            numInputs_->setValue(settings->getValue("Audio/channelsIn").toInt());

            but = new QToolButton(this);
            lh->addWidget(but);
            but->setText("D");
            but->setToolTip(tr("Set default value"));
            connect(but, SIGNAL(clicked()), this, SLOT(setDefaultChannelsIn_()));

        // channels out
        lh = new QHBoxLayout();
        l0->addLayout(lh);

            label = new QLabel(tr("output channels"), this);
            lh->addWidget(label);

            numOutputs_ = new QSpinBox(this);
            lh->addWidget(numOutputs_);
            numOutputs_->setRange(1, 256);
            numOutputs_->setValue(settings->getValue("Audio/channelsOut").toInt());

            but = new QToolButton(this);
            lh->addWidget(but);
            but->setText("D");
            but->setToolTip(tr("Set default value"));
            connect(but, SIGNAL(clicked()), this, SLOT(setDefaultChannelsOut_()));

        // --- test tone ---

        l0->addStretch(1);

        lh = new QHBoxLayout();
        l0->addLayout(lh);

            testButt_ = new QToolButton(this);
            lh->addWidget(testButt_);
            testButt_->setIcon(QIcon(":/icon/obj_soundsource.png"));
            testButt_->setText(tr("Test tone"));
            testButt_->setToolTip(tr("Test tone"));
            testButt_->setCheckable(true);
            testButt_->setEnabled(false);
            testButt_->setAutoFillBackground(true);
            connect(testButt_, SIGNAL(toggled(bool)),
                    this, SLOT(toggleTesttone_()));

            but = new QToolButton(this);
            lh->addWidget(but);
            but->setText("#");
            but->setToolTip(tr("Envelope"));
            but->setCheckable(true);
            connect(but, &QToolButton::toggled, [=]()
            {
                doEnv_ = but->isChecked();
            });

            auto freqSpin = new QSpinBox(this);
            lh->addWidget(freqSpin);
            freqSpin->setRange(1, 44000);
            freqSpin->setValue(freq_);
            freqSpin->setSuffix(" " + tr("hz"));
            connect(freqSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]()
            {
                freq_ = freqSpin->value();
            });

            auto volSpin = new QSpinBox(this);
            lh->addWidget(volSpin);
            volSpin->setRange(1,100);
            volSpin->setValue(vol_);
            volSpin->setSuffix(" " + tr("%"));
            connect(volSpin, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]()
            {
                vol_ = volSpin->value();
            });

        // -- ok / cancel --

        l0->addStretch(1);

        lh = new QHBoxLayout();
        l0->addLayout(lh);

            okButt_ = new QPushButton(this);
            lh->addWidget(okButt_);
            okButt_->setText(tr("Accept"));
            connect(okButt_, &QPushButton::pressed, [=]()
            {
                storeConfig_();
                close();
            });

            auto cancelButt = new QPushButton(this);
            lh->addWidget(cancelButt);
            cancelButt->setText(tr("Cancel"));
            connect(cancelButt, SIGNAL(pressed()), this, SLOT(close()));



    // blink timer

    timer_ = new QTimer(this);
    timer_->setInterval(1000 / 60);
    connect(timer_, &QTimer::timeout, [=]()
    {
        QPalette p(testButt_->palette());
        p.setColor(QPalette::Button, QColor(50, env_ * 255, 50));
        testButt_->setPalette(p);
    });

    checkDevices_();
    setWidgetChannelLimits_();
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
                             tr("Failed to initialize audio devices.\n%1").arg(e.what())
                             );
        return;
    }

    QString api = settings->getValue("Audio/api").toString();

    // fill api box
    for (uint i=0; i<devices_->numApis(); ++i)
    {
        auto inf = devices_->getApiInfo(i);
        apiBox_->addItem(inf->name, QVariant(i));
        if (inf->name == api)
            apiBox_->setCurrentIndex(i + 1);
    }


    fillDeviceBox_();
}


void AudioDialog::fillDeviceBox_()
{
    QString dev = settings->getValue("Audio/device").toString();

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

        if (inf->name == dev)
            deviceBox_->setCurrentIndex(i + 1);
    }
}

void AudioDialog::apiSelected_()
{
    fillDeviceBox_();
}

void AudioDialog::deviceSelected_()
{
    int idx = selectedDeviceIndex();
    bool works = idx >= 0 && devices_->getDeviceInfo(idx)->numOutputChannels;
    testButt_->setEnabled( works );
    okButt_->setEnabled( works );
    if (works)
    {
        if (!settings->contains("Audio/samplerate"))
            sampleRate_->setValue(devices_->getDeviceInfo(idx)->defaultSampleRate);
        if (!settings->contains("Audio/buffersize"))
            bufferSize_->setValue(devices_->getDeviceInfo(idx)->defaultBufferLength);
        if (!settings->contains("Audio/channelsIn"))
            numInputs_->setValue(devices_->getDeviceInfo(idx)->numInputChannels);
        if (!settings->contains("Audio/channelsOut"))
            numOutputs_->setValue(devices_->getDeviceInfo(idx)->numOutputChannels);
    }
    setWidgetChannelLimits_();
}

void AudioDialog::setWidgetChannelLimits_()
{
    int idx = selectedDeviceIndex();
    if (idx < 0)
    {
        numInputs_->setMaximum(256);
        numOutputs_->setMaximum(256);
        return;
    }
    numInputs_->setMaximum(devices_->getDeviceInfo(idx)->numInputChannels);
    numOutputs_->setMaximum(devices_->getDeviceInfo(idx)->numOutputChannels);
}

void AudioDialog::setDefaultSamplerate_()
{
    int idx = selectedDeviceIndex();
    if (idx >= 0)
    {
        sampleRate_->setValue(devices_->getDeviceInfo(idx)->defaultSampleRate);
    }
}


void AudioDialog::setDefaultBuffersize_()
{
    int idx = selectedDeviceIndex();
    if (idx >= 0)
    {
        bufferSize_->setValue(devices_->getDeviceInfo(idx)->defaultBufferLength);
    }
}

void AudioDialog::setDefaultChannelsIn_()
{
    int idx = selectedDeviceIndex();
    if (idx >= 0)
    {
        numInputs_->setValue(devices_->getDeviceInfo(idx)->numInputChannels);
    }
}

void AudioDialog::setDefaultChannelsOut_()
{
    int idx = selectedDeviceIndex();
    if (idx >= 0)
    {
        numOutputs_->setValue(devices_->getDeviceInfo(idx)->numOutputChannels);
    }
}

int AudioDialog::selectedDeviceIndex() const
{
    int idx = deviceBox_->currentIndex();
    if (idx < 1)
        return -1;

    return deviceBox_->itemData(idx).toInt();
}

void AudioDialog::storeConfig_()
{
    int idx = apiBox_->currentIndex();
    settings->setValue("Audio/api", idx < 1? "None" : devices_->getApiInfo(idx-1)->name );
    idx = deviceBox_->currentIndex();
    settings->setValue("Audio/device", idx < 1? "None" : devices_->getDeviceInfo(idx-1)->name );
    settings->setValue("Audio/samplerate", sampleRate_->value());
    settings->setValue("Audio/buffersize", bufferSize_->value());
    settings->setValue("Audio/channelsIn", numInputs_->value());
    settings->setValue("Audio/channelsOut", numOutputs_->value());
}



void AudioDialog::toggleTesttone_()
{
    if (testButt_->isChecked())
    {
        startTone_();
    }
    else
        stopTone_();
}

void AudioDialog::startTone_()
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
    conf.setNumChannelsIn(numInputs_->value());
    conf.setNumChannelsOut(numOutputs_->value());
    conf.setBufferSize(bufferSize_->value());
    conf.setSampleRate(sampleRate_->value());

    env_ = 0.f;

    device_->setCallback([=](const F32*, F32* out)
    {
#if (1)
        for (uint i=0; i<conf.bufferSize(); ++i)
        {
            F32 sam = 0.01f * vol_ * sin(phase_ * TWO_PI);

            if (doEnv_)
            {
                sam *= env_;
                if (env_ < 0.001f)
                    env_ = 1.f;
                else
                    env_ *= (1.f - conf.sampleRateInv() * 10.f);
            }

            for (uint j=0; j<conf.numChannelsOut(); ++j)
                *out++ = sam;

            realfreq_ += conf.sampleRateInv() * 100.f * (freq_ - realfreq_);

            phase_ += conf.sampleRateInv() * realfreq_;
            if (phase_ > 1.f)
                phase_ -= 2.f;
            else if (phase_ < -1.f)
                phase_ += 2.f;
        }
#else
        // test for consistency of translation between SamplePos and Double

        static SamplePos pos = 0;

        for (uint i=0; i<conf.bufferSize(); ++i)
        {
            Double phase = (Double)(pos + i) * conf.sampleRateInv() * realfreq_;

            F32 sam = 0.01f * vol_ * sin(phase * TWO_PI);

            if (doEnv_)
            {
                sam *= env_;
                if (env_ < 0.001f)
                    env_ = 1.f;
                else
                    env_ *= (1.f - conf.sampleRateInv() * 10.f);
            }

            for (uint j=0; j<conf.numChannelsOut(); ++j)
                *out++ = sam;

            realfreq_ += conf.sampleRateInv() * 100.f * (freq_ - realfreq_);
        }
        pos += conf.bufferSize();
#endif
    });

    try
    {
        device_->init(idx, conf);
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to init audio device '%1'.\n%2")
                             .arg(inf->name)
                             .arg(e.what())
                             );
        testButt_->setChecked(false);
        return;
    }


    try
    {
        device_->start();
        timer_->start();
    }
    catch (AudioException& e)
    {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to start audio device '%1'.\n%2")
                             .arg(inf->name)
                             .arg(e.what())
                             );
        device_->close();
        testButt_->setChecked(false);
        return;
    }
}


void AudioDialog::stopTone_()
{
    timer_->stop();
    if (device_)
    {
        device_->stop();
        device_->close();
    }
}



} // namespace GUI
} // namespace MO
