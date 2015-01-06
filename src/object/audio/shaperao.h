/** @file shaperao.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 08.12.2014</p>
*/

#ifndef MOSRC_OBJECT_AUDIO_SHAPERAO_H
#define MOSRC_OBJECT_AUDIO_SHAPERAO_H

#include "object/audioobject.h"

namespace MO {

class ShaperAO : public AudioObject
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(ShaperAO)
    ~ShaperAO();

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter * p) Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

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

#endif // MOSRC_OBJECT_AUDIO_SHAPERAO_H
