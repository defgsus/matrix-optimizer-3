/** @file synthesizer.h

    @brief Basic, non-graphic synthesizer object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MO_DISABLE_SPATIAL

#ifndef MOSRC_OBJECT_SYNTHESIZER_H
#define MOSRC_OBJECT_SYNTHESIZER_H

#include <vector>
#include <deque>

#include "object.h"

namespace PPP_NAMESPACE { class Parser; }
namespace MO {
namespace AUDIO { class SynthVoice; }

class SynthSetting;

class Synthesizer : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Synthesizer);

    virtual Type type() const Q_DECL_OVERRIDE { return T_SOUND_OBJECT; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void setSampleRate(uint samplerate) Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint numThreads) Q_DECL_OVERRIDE;


    virtual void calculateSoundSourceTransformation(
                                        const TransformationBuffer * objectTransformation,
                                        const QList<AUDIO::SpatialSoundSource*>&,
                                        const RenderTime& time)
                                            Q_DECL_OVERRIDE;
    virtual void calculateSoundSourceBuffer(const QList<AUDIO::SpatialSoundSource*>,
                                            const RenderTime& time)
                                            Q_DECL_OVERRIDE;

    void panic();

protected:

    void updateAudioSources_();
    void updateBuffers_();
    void updatePosParser_();
    //void updateBufferSize_(uint bufferSize, uint thread);
    void setCallbacks_();

    struct VoicePos_
    {
        uint sample;
        Double sceneTime;
        Mat4 trans;
    };

    struct VoiceEqu_
    {
        VoiceEqu_();
        ~VoiceEqu_();
        void initParser_(PPP_NAMESPACE::Parser*);
        void feedParser_(Double x, Double y, Double z, const AUDIO::SynthVoice& v);
        PPP_NAMESPACE::Parser
            *parserX, *parserY, *parserZ;
        Double note, freq, vel, time, timer, x, y, z;
    };

    SynthSetting * synth_;

    /** [audiosource] */
    std::vector<Mat4> audioPos_;
    /** [audiosource][voice-in-time] */
    std::vector<std::deque<VoicePos_>> audioPosFifo_;
    /** [thread] */
    std::vector<VoiceEqu_> voiceEqu_;
    volatile bool doPanic_;

    ParameterSelect
        * p_polyAudio_,
        * p_posEqu_;

    ParameterFloat
        * p_audioX_,
        * p_audioY_,
        * p_audioZ_;

    ParameterText
        * p_equX_,
        * p_equY_,
        * p_equZ_;

};

} // namespace MO


#endif // MOSRC_OBJECT_SYNTHESIZER_H

#endif // #ifndef MO_DISABLE_SPATIAL
