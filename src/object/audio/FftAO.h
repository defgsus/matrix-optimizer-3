/** @file fftao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

//#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_AUDIO_FFTAO_H
#define MOSRC_OBJECT_AUDIO_FFTAO_H

#include "object/AudioObject.h"
#include "object/interface/ValueFloatInterface.h"
#include "object/interface/ValueFloatMatrixInterface.h"

namespace MO {

class FftAO : public AudioObject
            , public ValueFloatInterface
            , public ValueFloatMatrixInterface
{
public:

    enum FftType
    {
        FT_AUDIO_2_FFT,
        FT_FFT_2_AUDIO,
        FT_AUDIO_2_AMPPHASE,
    };

    enum MatrixMode
    {
        MM_OFF,
        MM_LINEAR,
        MM_SPLIT
    };

    MO_OBJECT_CONSTRUCTOR(FftAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

    virtual QString getOutputName(SignalType, uint channel) const override;
    virtual QString infoString() const override;

    size_t fftSize() const;
    size_t delayInSamples() const;

    virtual Double valueFloat(
            uint channel, const RenderTime &time) const override;
    virtual FloatMatrix valueFloatMatrix(
            uint channel, const RenderTime &time) const override;
    virtual bool hasFloatMatrixChanged(
            uint channel, const RenderTime &time) const override;

protected:

    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE;

    virtual void processAudio(const RenderTime&) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIO_FFTAO_H

//#endif // #ifndef MO_DISABLE_EXP
