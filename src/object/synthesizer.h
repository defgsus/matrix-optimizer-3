/** @file synthesizer.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_OBJECT_SYNTHESIZER_H
#define MOSRC_OBJECT_SYNTHESIZER_H

#include "object.h"

namespace MO {
namespace AUDIO { class Synth; template <typename F> class FloatGate; }

class Synthesizer : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Synthesizer);
    ~Synthesizer();

    virtual Type type() const Q_DECL_OVERRIDE { return T_OBJECT; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void createAudioSources() Q_DECL_OVERRIDE;

    virtual void setSampleRate(uint samplerate) Q_DECL_OVERRIDE;

    virtual void updateAudioTransformations(Double time, uint thread) Q_DECL_OVERRIDE;
    virtual void updateAudioTransformations(Double time, uint blockSize, uint thread)
                                                                    Q_DECL_OVERRIDE;

    virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;
signals:

public slots:

private:

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
        * p_waveform_,
        * p_filterType_,
        * p_combinedUnison_;

    AUDIO::Synth * synth_;
    AUDIO::FloatGate<Double> * gate_;

    AUDIO::AudioSource * audio_;
};

} // namespace MO


#endif // MOSRC_OBJECT_SYNTHESIZER_H
