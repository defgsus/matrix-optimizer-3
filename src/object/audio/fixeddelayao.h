/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_FIXEDDELAYAO_H
#define MOSRC_OBJECT_AUDIO_FIXEDDELAYAO_H

#include "object/audioobject.h"

namespace MO {

class FixedDelayAO : public AudioObject
{
public:
    MO_OBJECT_CONSTRUCTOR(FixedDelayAO)

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint count) Q_DECL_OVERRIDE;
    virtual QString infoString() const override;
    virtual void setAudioBuffers(uint thread, uint bufferSize,
                                 const QList<AUDIO::AudioBuffer*>& inputs,
                                 const QList<AUDIO::AudioBuffer*>& outputs) override;

protected:

    virtual void processAudio(const RenderTime& time) Q_DECL_OVERRIDE;
private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_AUDIO_FIXEDDELAYAO_H
