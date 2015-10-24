/** @file bandfilterbankao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 19.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_BANDFILTERBANKAO_H
#define MOSRC_OBJECT_AUDIO_BANDFILTERBANKAO_H

#include "object/audioobject.h"

namespace MO {

class BandFilterBankAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(BandFilterBankAO)
    ~BandFilterBankAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter*) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

protected:

    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs)
                                                            Q_DECL_OVERRIDE;

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_BANDFILTERBANKAO_H
