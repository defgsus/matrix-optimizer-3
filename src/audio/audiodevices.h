/** @file audiodevices.h

    @brief Information center and initialiser for audio devices.

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/22/2014</p>
*/

#ifndef MOSRC_AUDIO_AUDIODEVICES_H
#define MOSRC_AUDIO_AUDIODEVICES_H

#include <iostream>

#include <QString>
#include <QList>

#include "types/int.h"

namespace MO {
namespace AUDIO {

/** @brief Information center and initialiser for audio devices.

    Currently simply wraps portaudio. */
class AudioDevices
{
    // for pa_initialized_
    friend class AudioDevice;

    public:

    struct ApiInfo
    {
        QString name;
        uint numDevices;
    };

    struct DeviceInfo
    {
        QString name;
        uint apiIndex,
             numInputChannels,
             numOutputChannels,
             defaultSampleRate,
             defaultBufferLength;
    };

    // --------------- ctor --------------

    AudioDevices();
    ~AudioDevices();

    // ---------- info -------------------

    /** call this once */
    bool checkDevices();

    uint numApis() const;
    uint numDevices() const;

    const ApiInfo * getApiInfo(uint index) const;
    const DeviceInfo * getDeviceInfo(uint index) const;

    void dump_info(std::ostream& = std::cout) const;

    // _________ HIDDEN AREA _____________

private:

    uint numDevices_, numApis_;
    std::vector<DeviceInfo> dev_infos_;
    std::vector<ApiInfo> api_infos_;

    // global flag to avoid multiple calls to Pa_Initialize()
    static bool pa_initialized_;
};



} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_AUDIODEVICES_H
