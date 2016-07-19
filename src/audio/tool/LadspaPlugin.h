/** @file ladspaplugin.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#ifndef MOSRC_AUDIO_TOOL_LADSPAPLUGIN_H
#define MOSRC_AUDIO_TOOL_LADSPAPLUGIN_H


#include <QString>

#include "audio/3rd/ladspa.h"
#include "types/Refcounted.h"
#include "types/float.h"

namespace MO {
namespace IO { class LadspaLoader; }
namespace AUDIO {

class AudioBuffer;

/** A LADSPA plugin wrapper */
class LadspaPlugin : public RefCounted
{
public:

    /** Info about a control port */
    struct ControlInfo
    {
        QString name;
        bool isBool,
             isInteger,
             isMinLimit,
             isMaxLimit;
        float minValue,
              maxValue,
              defaultValue;
    };

    /** Returns true when the device has been loaded */
    bool isLoaded() const;
    bool isInitialized() const;

    QString idName() const;
    QString name() const;
    const QString& filename() const { return p_filename_; }

    /** Returns information about the plugin, multiline */
    QString infoString();

    /** Input and output channels */
    size_t numPorts() const;

    // --- below settings are only valid after initialize() ---

    size_t numInputs() const { return p_audioIns_.size(); }
    size_t numOutputs() const { return p_audioOuts_.size(); }
    size_t numControlInputs() const { return p_controlIns_.size(); }
    size_t numControlOutputs() const { return p_controlOuts_.size(); }

    QList<QString> inputNames() const;
    QList<QString> outputNames() const;

    /** The bufferSize as set by initialize() */
    size_t blockSize() const { return p_bsize_; }
    size_t sampleRate() const { return p_sr_; }

    /** Returns the info for input control port @p index [0, numControlInputs()-1] */
    ControlInfo getControlInfo(size_t index) const;

    // ----------------- processing --------------------

    bool initialize(size_t blockSize, size_t sampleRate);

    /** Sets the @p index'th [0, numControlInputs()-1] control input value.
        No range or sanity checking for value. */
    void setControlValue(size_t index, F32 value);

    /** Plugin must be initialized and blockSizes must match */
    void process(const QList<AudioBuffer*> & ins, const QList<AudioBuffer*> & outs);

private:
    friend class IO::LadspaLoader;

    LadspaPlugin();
    ~LadspaPlugin();

    // disable copy
    LadspaPlugin(const LadspaPlugin&);
    void operator = (const LadspaPlugin&);

    void p_getPorts_();
    void p_connect_();
    void p_shutDown_();

    struct Port;
    std::vector<Port*>
        p_ports_,
        p_audioIns_, p_audioOuts_,
        p_controlIns_, p_controlOuts_;
    size_t p_sr_, p_bsize_;
    bool p_activateCalled_;

    // -- things managed by IO::LadspaLoader --

    LADSPA_Handle p_handle_;
    const LADSPA_Descriptor * p_desc_;
    QString p_filename_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_TOOL_LADSPAPLUGIN_H

#endif // #ifndef MO_DISABLE_LADSPA
