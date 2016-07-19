/** @file mididevice.h

    @brief Midi device wrapper

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_AUDIO_MIDIDEVICE_H
#define MOSRC_AUDIO_MIDIDEVICE_H

#include <QString>

#include "MidiEvent.h"

namespace MO {
namespace AUDIO {

class MidiDevice
{
public:

    MidiDevice();
    ~MidiDevice();

    // ------------ init -------------------

    /** Tries to open the device id as input.
        Throws AudiException on errors */
    void openInput(uint id);
    /** Just closes the device - no error checking */
    void close();

    /** Initializes a device from the stored settings.
        Returns true when this worked.
        Returns false otherwise or if no settings are made yet.
        Shows an error dialog on api failure.
        @see isMidiConfigured() */
    bool openInputFromSettings();

    // --------------- getter --------------

    /** Returns true when application settings for midi input have been set */
    bool isMidiInputConfigured() const;

    bool isOpen() const;

    const QString& apiName() const { return apiName_; }
    const QString& deviceName() const { return deviceName_; }

    int deviceId() const { return id_; }

    // -------------- input ----------------

    bool isInputEvent() const;

    MidiEvent read();

private:

    QString apiName_, deviceName_;
    int id_;
    void * stream_, * events_;
    uint numEvents_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_MIDIDEVICE_H
