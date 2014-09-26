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
        * p_note_;

    ParameterFloat
        * p_gate_,
        * p_notesPerOct_,
        * p_baseFreq_,
        * p_attack_,
        * p_decay_,
        * p_sustain_,
        * p_release_,
        * p_pulseWidth_;

    ParameterSelect
        * p_waveform_;

    AUDIO::Synth * synth_;
    AUDIO::FloatGate<Double> * gate_;

    AUDIO::AudioSource * audio_;
};

} // namespace MO


#endif // MOSRC_OBJECT_SYNTHESIZER_H
