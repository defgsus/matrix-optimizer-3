/** @file synthsetting.h

    @brief Wrapper for AUDIO::Synth and it's parameters for an Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.09.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_SYNTHSETTING_H
#define MOSRC_OBJECT_UTIL_SYNTHSETTING_H

#include <vector>

#include <QObject>

#include "object/object_fwd.h"
#include "types/time.h"

namespace MO {
namespace AUDIO { class Synth; class SynthVoice; template <typename F> class FloatGate; }


class SynthSetting : public QObject
{
    Q_OBJECT
public:

    /** Each triggered voice contains a pointer to this field */
    struct VoiceData
    {
        /** Scene-time when the voice was started */
        Double timeStarted;
        /** Thread in which feedSynth() was called */
        uint thread;
        /** Freely chooseable user data */
        void * userData;
    };


    explicit SynthSetting(Object *parent);
    ~SynthSetting();

    // ---------- parameters -----------

    /** Creates the synth-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one AUDIO::Synth for an Object. */
    void createParameters(const QString& id_suffix);

    /** Call this in Object::onParameterChanged().
        Returns true when the Parameter belongs to this Synth. */
    bool onParameterChanged(Parameter * p);

    /** Call this in Object::onParametersLoaded() */
    void onParametersLoaded();

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    // ------------ getter ---------------

    AUDIO::Synth * synth() const { return synth_; }

    // ------------ runtime --------------

    /** Feeds all parameters at the given time to the synthesizer. */
    void updateSynthParameters(const MO::RenderTime & time);

    /** Feeds all parameters at the given time to the synthesizer and
        starts voices according to gate track.
        Subsequently AUDIO::Synth::process() can be called. */
    void feedSynthOnce(const MO::RenderTime & time);

    /** Feeds all parameters at the given time to the synthesizer and
        starts voices according to gate track.
        The gate track is checked for the whole audio block.
        @p samplePos is translated into seconds with the samplerate of the parent object.
        Subsequently AUDIO::Synth::process() can be called. */
    void feedSynth(const MO::RenderTime & time);

private:

    Object * o_;
    AUDIO::Synth * synth_;
    AUDIO::FloatGate<Double> * gate_;
    std::vector<VoiceData> voiceData_;

    ParameterInt
        * p_numVoices_,
        * p_note_,
        * p_filterOrder_,
        * p_numUnison_,
        * p_unisonNoteStep_;

    ParameterFloat
        * p_volume_,
        * p_gate_,
        * p_notesPerOct_,
        * p_baseFreq_,
        * p_unisonDetune_,
        * p_attack_,
        * p_decay_,
        * p_sustain_,
        * p_release_,
        * p_pulseWidth_,
        * p_filterFreq_,
        * p_filterReso_,
        * p_filterKeyFollow_,
        * p_filterEnv_,
        * p_filterEnvKeyFollow_,
        * p_fattack_,
        * p_fdecay_,
        * p_fsustain_,
        * p_frelease_;

    ParameterSelect
        * p_policy_,
        * p_waveform_,
        * p_filterType_,
        * p_combinedUnison_;

};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_SYNTHSETTING_H
