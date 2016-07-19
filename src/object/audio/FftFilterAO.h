/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/15/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FFTFILTERAO_H
#define MOSRC_OBJECT_AUDIO_FFTFILTERAO_H

#include "object/AudioObject.h"

namespace MO {

class FftFilterAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(FftFilterAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;
protected:

    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE;
    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FFTFILTERAO_H
