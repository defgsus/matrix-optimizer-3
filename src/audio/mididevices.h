/** @file mididevices.h

    @brief Midi devices info

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_AUDIO_MIDIDEVICES_H
#define MOSRC_AUDIO_MIDIDEVICES_H

#include <QString>
#include <QList>

namespace MO {
namespace AUDIO {


class MidiDevices
{
    // for pm_initialized_
    friend class MidiDevice;

public:

    struct DeviceInfo
    {
        int id;
        QString name, apiName;
        bool output;
    };


    MidiDevices();

    // ------------ init -------------------

    /** Call this to get all device infos */
    bool checkDevices();

    // --------------- getter --------------

    uint numApis() const { return apiNames_.count(); }
    uint numDevices() const { return devices_.count(); }

    const QList<QString> apiNames() const { return apiNames_; }
    const QList<DeviceInfo> deviceInfos() const { return devices_; }

private:

    /** Global check for Pm_Initialized() called */
    static bool pm_initialized_;

    QList<QString> apiNames_;
    QList<DeviceInfo> devices_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_MIDIDEVICES_H
