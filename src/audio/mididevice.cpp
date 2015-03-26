/** @file mididevice.cpp

    @brief Midi device wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <portmidi.h>

#include <QMessageBox>

#include "mididevice.h"
#include "mididevices.h"
#include "io/error.h"
#include "io/log.h"
#include "io/settings.h"

namespace MO {
namespace AUDIO {

MidiDevice::MidiDevice()
    :   id_         (-1),
        stream_     (0),
        numEvents_  (1024)
{

}

MidiDevice::~MidiDevice()
{
    if (isOpen())
        close();
}

bool MidiDevice::isOpen() const
{
    return (stream_ != 0);
}

void MidiDevice::openInput(uint id)
{
    MO_DEBUG_MIDI("MidiDevice::openInput(" << id << ")");

    // -- sanity first --

    if (isOpen())
    {
        MO_WARNING("attempt to open midi input which is already opened");
        return;
    }

    MidiDevices devs;
    devs.checkDevices();

    if (id >= devs.numDevices())
        MO_AUDIO_ERROR(LOGIC, "id " << id <<
              " out of range for midi devices " << devs.numDevices());

    if (devs.deviceInfos()[id].output)
        MO_AUDIO_ERROR(LOGIC,
              "attempt to open output midi device " << id << " as input");


    if (!MidiDevices::pm_initialized_)
    {
        MidiDevices::pm_initialized_ = true;
        Pm_Initialize();
    }

    PortMidiStream * stream;

    PmError err =
            Pm_OpenInput(&stream, id, 0, 0, 0, 0);

    if (err != pmNoError)
    {
        MO_AUDIO_ERROR(API, "Could not open midi input device\n"
                       << Pm_GetErrorText(err));
    }

    // okay

    id_ = id;
    stream_ = stream;
    apiName_ = devs.deviceInfos()[id_].apiName;
    deviceName_ = devs.deviceInfos()[id_].name;

    events_ = calloc(numEvents_, sizeof(PmEvent));

}

void MidiDevice::close()
{
    MO_DEBUG_MIDI("MidiDevice(" << id_ << ")::close()");

    if (!stream_)
    {
        MO_WARNING("attempt to close midi device which is not opened");
        return;
    }

    Pm_Close((PortMidiStream*)stream_);

    stream_ = 0;
    id_ = -1;
    apiName_.clear();
    deviceName_.clear();

    free(events_);
}


bool MidiDevice::isMidiInputConfigured() const
{
    return settings()->getValue("MidiIn/device").toString().size();
}

bool MidiDevice::openInputFromSettings()
{
    QString devname = settings()->getValue("MidiIn/device").toString();
    if (devname.isEmpty())
    {
        QMessageBox::critical(0, QMessageBox::tr("midi input"),
                              QMessageBox::tr("Midi input has not been configured"));
        return false;
    }

    // find today's id
    MidiDevices devs;
    devs.checkDevices();

    int id = devs.idForName(devname, false);


    if (id < 0)
    {
        QMessageBox::warning(0, QMessageBox::tr("midi input"),
                 QMessageBox::tr("The configured midi device '%1' could not be found.")
                 .arg(devname)
                         );
        return false;
    }

    try
    {
        openInput(id);
    }
    catch (Exception & e)
    {
        QMessageBox::critical(0, QMessageBox::tr("midi input"),
            QMessageBox::tr(
                "Sorry, but there was an error opening the midi input device '%1'\n%2")
                              .arg(devname).arg(e.what()));
        return false;
    }

    return true;
}








bool MidiDevice::isInputEvent() const
{
    if (!stream_)
        return false;

    int e = Pm_Poll(stream_);
    if (e < 0)
    {
        MO_WARNING("Pm_Poll() returned error " << Pm_GetErrorText((PmError)e));
        return false;
    }

    return e > 0;
}

MidiEvent MidiDevice::read()
{
    int num = Pm_Read(stream_, (PmEvent*)events_, 1);

    //MO_DEBUG_MIDI("MidiDevice(" << id_ << ")::read() " << num << " events");

    if (num < 0)
    {
        MO_WARNING("Pm_Read() returned error " << Pm_GetErrorText((PmError)num));
        return MidiEvent();
    }

    //MO_DEBUG_MIDI("MidiDevice(" << id_ << ")::read() event = "
    //              << QString("%1").arg(((PmEvent*)events_)->message, 8, 16, QChar('0')));

    return MidiEvent(uint32_t( ((PmEvent*)events_)->message ));
}



} // namespace AUDIO
} // namespace MO
