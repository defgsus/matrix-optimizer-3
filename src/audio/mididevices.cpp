/** @file mididevices.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <portmidi.h>

#include "mididevices.h"

namespace MO {
namespace AUDIO {



bool MidiDevices::pm_initialized_ = false;

MidiDevices::MidiDevices()
{
    if (!pm_initialized_)
    {
        pm_initialized_ = true;
        Pm_Initialize();
    }
}

bool MidiDevices::checkDevices()
{
    apiNames_.clear();
    devices_.clear();

    const int count = Pm_CountDevices();
    if (count < 1)
        return false;

    const PmDeviceInfo * info;

    for (int i=0; i<count; ++i)
    {
        info = Pm_GetDeviceInfo(i);

        // save api info
        const QString api = info->interf;
        if (!apiNames_.contains(api))
            apiNames_.append(api);

        // save device info
        DeviceInfo inf;
        inf.id = i;
        inf.name = info->name;
        inf.input = info->input;
        inf.output = info->output;
        devices_.append(inf);
    }

    return true;
}


} // namespace AUDIO
} // namespace MO
