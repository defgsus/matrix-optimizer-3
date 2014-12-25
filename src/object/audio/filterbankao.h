/** @file filterbankao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FILTERBANKAO_H
#define MOSRC_OBJECT_AUDIO_FILTERBANKAO_H

#include "object/audioobject.h"

namespace MO {

class FilterBankAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(FilterBankAO)
    ~FilterBankAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter*) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;

    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;

protected:

    virtual void setAudioBuffers(uint thread,
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

#endif // MOSRC_OBJECT_AUDIO_FILTERBANKAO_H
