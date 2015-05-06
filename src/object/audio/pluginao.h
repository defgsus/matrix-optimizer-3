/** @file pluginao.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#ifndef MOSRC_OBJECT_AUDIO_PLUGINAO_H
#define MOSRC_OBJECT_AUDIO_PLUGINAO_H

#include "object/audioobject.h"

namespace MO {
namespace AUDIO { class LadspaPlugin; }

/** Audio plugin wrapper.
    Currently only based on AUDIO::LadspaPlugin */
class PluginAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(PluginAO);
    ~PluginAO();

    /** Maximum plugin parameters to create */
    static const size_t maxParams;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual QString getAudioInputName(uint channel) const Q_DECL_OVERRIDE;
    virtual QString getAudioOutputName(uint channel) const Q_DECL_OVERRIDE;

    /** Sets the current plugin, or NULL to disable
        Only callable by GUI thread! */
    void setPlugin(AUDIO::LadspaPlugin * );

protected:

    virtual void processAudio(uint bsize, SamplePos pos, uint thread) Q_DECL_OVERRIDE;

private:

    class Private;
    Private *p_;
};

} // namepsace MO

#endif // MOSRC_OBJECT_AUDIO_PLUGINAO_H

#endif // #ifndef MO_DISABLE_LADSPA
