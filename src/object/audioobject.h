/** @file audioobject.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

#ifndef AUDIOOBJECT_H
#define AUDIOOBJECT_H

#include "object.h"

namespace MO {
namespace AUDIO { class AudioBuffer; }

class AudioObject : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(AudioObject)
    ~AudioObject();

    virtual Type type() const Q_DECL_OVERRIDE { return T_AUDIO_OBJECT; }
    virtual bool isAudioObject() const Q_DECL_OVERRIDE { return true; }

    /** Returns the desired number of audio output channels */
    uint numAudioOutputs() const;

    /** Processes dsp data.
        Inputs and outputs must have the same buffer size */
    void processAudioBase(const QList<AUDIO::AudioBuffer*>& inputs,
                          const QList<AUDIO::AudioBuffer*>& outputs,
                          uint bufferSize, SamplePos pos, uint thread);

protected:

    /** Process dsp data here.
        Inputs and outputs have the same buffer size */
    virtual void processAudio(const QList<AUDIO::AudioBuffer*>& inputs,
                              const QList<AUDIO::AudioBuffer*>& outputs,
                              uint bufferSize, SamplePos pos, uint thread) = 0;

    void setNumberAudioOutputs(uint num);

    /** Copies all input data to output data. */
    void bypass(const QList<AUDIO::AudioBuffer*>& inputs,
                const QList<AUDIO::AudioBuffer*>& outputs);

private:
    class PrivateAO;
    PrivateAO * p_ao_;
};

} // namespace MO


#endif // AUDIOOBJECT_H
