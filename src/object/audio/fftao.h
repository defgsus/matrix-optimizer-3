/** @file fftao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 05.12.2014</p>
*/

#ifndef MO_DISABLE_EXP

#ifndef MOSRC_OBJECT_AUDIO_FFTAO_H
#define MOSRC_OBJECT_AUDIO_FFTAO_H

#include <QObject>

#include "object/audioobject.h"

namespace MO {

class FftAO : public AudioObject
{
    Q_OBJECT
public:

    enum FftType
    {
        FT_FFT,
        FT_IFFT
    };


    MO_OBJECT_CONSTRUCTOR(FftAO)
    ~FftAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

protected:

    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE;

    virtual void processAudio(uint bufferSize, SamplePos pos, uint thread)
                                                            Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_OBJECT_AUDIO_FFTAO_H

#endif // #ifndef MO_DISABLE_EXP
