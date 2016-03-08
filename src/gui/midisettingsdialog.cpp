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
#include "audio/audiodevice.h"
#include "audio/tool/synth.h"
#include "io/error.h"
#include "io/applicationtime.h"
#include "io/settings.h"
#include "io/log_midi.h"
#include "tool/locklessqueue.h"

namespace MO {
namespace GUI {

// Enables polyphonic output of Synth
#define MO_MS_SYNTH_POLYOUT


class MidiSettingsDialog::Private
{
public:

    Private()
        : device(new AUDIO::AudioDevice()),
          synth(new AUDIO::Synth())
    { }

    ~Private()
    {
        delete synth;
        delete device;
    }

    AUDIO::AudioDevice * device;
    AUDIO::Synth * synth;
    std::vector<F32> buffer;
#ifdef MO_MS_SYNTH_POLYOUT
    std::vector<F32*> buffers;
#endif
    LocklessQueue<AUDIO::MidiEvent> queue;
};


MidiSettingsDialog::MidiSettingsDialog(QWidget *parent)
    : QDialog   (parent),
      curId_    (-1),
      curDevice_(0),
      devices_  (0),
      timer_    (new QTimer(this)),
      p_        (0)
{
    setObjectName("_MidiSettingsDialog");
    setWindowTitle(tr("Midi settings"));
    setWindowIcon(QIcon(":/icon/midi.png"));
    setMinimumSize(640,280);

    timer_->setSingleShot(false);
    timer_->setInterval(1000 / 30);
    connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer_()));

    createWidgets_();

    loadSettings_();
}

MidiSettingsDialog::~MidiSettingsDialog()
{
    if (p_ && p_->device->isPlaying())
        p_->device->close();
    delete p_;
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

            auto lv2 = new QVBoxLayout();
            lh->addLayout(lv2);

                butTest_ = new QToolButton(this);
                lv2->addWidget(butTest_);
                butTest_->setText(tr("test input"));
                butTest_->setCheckable(true);
                connect(butTest_, SIGNAL(toggled(bool)), this, SLOT(onTest_(bool)));

                butTestSynth_ = new QToolButton(this);
                lv2->addWidget(butTestSynth_);
                butTestSynth_->setText(tr("test synthesizer"));
                butTestSynth_->setCheckable(true);
                connect(butTestSynth_, SIGNAL(toggled(bool)), this, SLOT(onTestSynth_(bool)));

                lv2->addStretch(1);

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
    butTestSynth_->setEnabled(curId_ >= 0
                              && AUDIO::AudioDevice::isAudioConfigured());
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
    butTestSynth_->setEnabled(!go);
    startTest_(go, false);
}

void MidiSettingsDialog::onTestSynth_(bool go)
{
    butTest_->setDown(go);
    butTest_->setEnabled(!go);
    startTest_(go, true);
}

void MidiSettingsDialog::startTest_(bool go, bool audio)
{
    if (go)
    {
        curDevice_ = new AUDIO::MidiDevice();
        try
        {
            curDevice_->openInput(curId_);
            if (audio)
                startAudio_(true);
            timer_->start();
            return;
        }
        catch (Exception& e)
        {
            QMessageBox::critical(this, tr("midi"),
                                  tr("Could not open the midi input device\n%1")
                                  .arg(e.what()));
            butTest_->setDown(false);
            butTestSynth_->setDown(false);
        }
    }

    startAudio_(false);
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

        if (p_ && p_->device->isPlaying())
        {
            // send all midi events to audio thread
            p_->queue.produce(e);
        }

    }
}

void MidiSettingsDialog::saveSettings_()
{
    MO_DEBUG_MIDI("saving midi settings " << curApiName_ << "/" << curDeviceName_);

    if (curId_ < 0 || curDeviceName_.isEmpty())
    {
        settings()->setValue("MidiIn/api", "");
        settings()->setValue("MidiIn/device", "");
    }
    else
    {
        settings()->setValue("MidiIn/api", curApiName_);
        settings()->setValue("MidiIn/device", curDeviceName_);
    }
}

void MidiSettingsDialog::loadSettings_()
{
    curApiName_ = settings()->getValue("MidiIn/api").toString();
    curDeviceName_ = settings()->getValue("MidiIn/device").toString();

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

void MidiSettingsDialog::startAudio_(bool start)
{
    if (!start && p_)
    {
        if (p_->device->isPlaying())
            p_->device->close();
        return;
    }

    if (!p_)
    {
        p_ = new Private();

        p_->device->setCallback(
            [this](const F32 *, F32 * out, const AUDIO::AudioDevice::StreamTime&)
        {
            AUDIO::MidiEvent event;
            while (p_->queue.consume(event))
            {
                if (event.command() == AUDIO::MidiEvent::C_NOTE_ON)
                    p_->synth->noteOn(event.key(), (Float)event.velocity()/127);
                else if (event.command() == AUDIO::MidiEvent::C_NOTE_OFF)
                    p_->synth->noteOff(event.key());
                else if (event.command() == AUDIO::MidiEvent::C_CONTROL_CHANGE)
                {
                    if (event.controller() == 1)
                         p_->synth->setFilterFrequency(
                             100.f + (Float)event.value() / 127 * 5000.f);
                }

            }

#ifndef MO_MS_SYNTH_POLYOUT

            // monophonic output
            F32 * buf = &p_->buffer[0];
            p_->synth->process(buf, p_->buffer.size());

            for (uint i=0; i<p_->buffer.size(); ++i, ++buf)
                for (uint j=0; j<p_->device->numOutputChannels(); ++j, ++out)
                    *out = *buf;
#else
            // polyphonic output
            p_->synth->process(&p_->buffers[0], p_->device->bufferSize());

            // mix back together
            memset(out, 0, sizeof(F32) * p_->device->bufferSize() * p_->device->numOutputChannels());
            for (uint i=0; i<p_->device->bufferSize(); ++i)
            {
                for (uint j=0; j<p_->synth->numberVoices(); ++j)
                {
                    const F32 * buf = p_->buffers[j];
                    out[i * p_->device->numOutputChannels()
                          + (j % p_->device->numOutputChannels())] += buf[i];
                }
            }
#endif

        });
    }

    if (p_->device->initFromSettings())
    {
        p_->synth->setNumberVoices(16);
        p_->synth->setSustain(1);
        p_->synth->setRelease(3);
        p_->synth->setWaveform(AUDIO::Waveform::T_TRIANGLE);
        p_->synth->setFilterFrequency(100);
        p_->synth->setFilterType(AUDIO::MultiFilter::T_24_LOW);
        p_->synth->setFilterResonance(0.6);
        p_->synth->setFilterKeyFollower(0.5);
        p_->synth->setFilterEnvelopeKeyFollower(2);
        p_->synth->setFilterDecay(3.5);

        p_->synth->setFilterSustain(0.5);
#ifndef MO_MS_SYNTH_POLYOUT
        p_->buffer.resize(p_->device->bufferSize());
#else
        p_->buffer.resize(p_->device->bufferSize() * p_->synth->numberVoices());
        p_->buffers.resize(p_->synth->numberVoices());
        for (uint i=0; i<p_->synth->numberVoices(); ++i)
            p_->buffers[i] = &p_->buffer[i * p_->device->bufferSize()];
#endif
        p_->device->start();
    }
}


} // namespace GUI
} // namespace MO
