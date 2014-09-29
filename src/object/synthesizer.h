/** @file synthesizer.h

    @brief Basic, non-graphic synthesizer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_OBJECT_SYNTHESIZER_H
#define MOSRC_OBJECT_SYNTHESIZER_H

#include <vector>
#include <deque>

#include "object.h"

namespace MO {

class SynthSetting;

class Synthesizer : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Synthesizer);

    virtual Type type() const Q_DECL_OVERRIDE { return T_OBJECT; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void createAudioSources() Q_DECL_OVERRIDE;

    virtual void setSampleRate(uint samplerate) Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint numThreads) Q_DECL_OVERRIDE;
    virtual void setBufferSize(uint bufferSize, uint thread) Q_DECL_OVERRIDE;

    virtual void updateAudioTransformations(Double time, uint thread) Q_DECL_OVERRIDE;
    virtual void updateAudioTransformations(Double time, uint blockSize, uint thread)
                                                                    Q_DECL_OVERRIDE;

    virtual void performAudioBlock(SamplePos pos, uint thread) Q_DECL_OVERRIDE;

protected:

    void updateAudioSources_();
    void setCallbacks_();

    struct VoicePos_
    {
        uint sample;
        Double sceneTime;
        Mat4 trans;
    };

    SynthSetting * synth_;

    QList<AUDIO::AudioSource*> audios_;

    /** [thread][audiosource] */
    std::vector<std::vector<F32*>> audioBuffers_;
    /** [thread][audiosource] */
    std::vector<std::vector<Mat4>> audioPos_;
    /** [thread][audiosource][voice-in-time] */
    std::vector<std::vector<std::deque<VoicePos_>>> audioPosFifo_;

    ParameterSelect
        * p_polyAudio_;

    ParameterFloat
        * p_audioX_,
        * p_audioY_,
        * p_audioZ_;
};

} // namespace MO


#endif // MOSRC_OBJECT_SYNTHESIZER_H
